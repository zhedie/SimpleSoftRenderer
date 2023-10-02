
#include "imgui.h"
#include "imgui_impl_vulkan.h"

#include "image.h"
#include "application.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static uint32_t getVulkanMemoryType(VkMemoryPropertyFlags properties, uint32_t type_bits) {
    VkPhysicalDeviceMemoryProperties prop;
    vkGetPhysicalDeviceMemoryProperties(Application::getPhysicalDevice(), &prop);
    for (uint32_t i = 0; i < prop.memoryTypeCount; ++i) {
        if ((prop.memoryTypes[i].propertyFlags & properties) == properties && type_bits & (1 << i)) {
            return i;
        }

    }
    return 0xffffffff;
}

static uint32_t bytesPerPixel(ImageFormat format) {
    switch (format) {
        case ImageFormat::RGBA:
            return 4;
        case ImageFormat::RGBA32F:
            return 16;
    }
    return 0;
}

static VkFormat toVulkanFormat(ImageFormat format) {
    switch (format) {
        case ImageFormat::RGBA:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case ImageFormat::RGBA32F:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
    }
    return (VkFormat)0;
}

void Image::allocateMemory(uint64_t size) {
    VkDevice device = Application::getDevice();

    VkResult err;

    VkFormat vulkanFormat = toVulkanFormat(this->format);

    // Create Image
    {
        VkImageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = vulkanFormat;
        info.extent.width = this->width;
        info.extent.height = this->height;
        info.extent.depth = 1;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        err = vkCreateImage(device, &info, nullptr, &image);
        check_vk_result(err);
        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(device, image, &req);
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = req.size;
        allocInfo.memoryTypeIndex = getVulkanMemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, req.memoryTypeBits);
        err = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
        check_vk_result(err);
        err = vkBindImageMemory(device, image, memory, 0);
        check_vk_result(err);
    }

    // Create Image View
    {
        VkImageViewCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image = image;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = vulkanFormat;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.layerCount = 1;
        err = vkCreateImageView(device, &info, nullptr, &imageView);
        check_vk_result(err);
    }

    // Create sampler
    {
        VkSamplerCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_LINEAR;
        info.minFilter = VK_FILTER_LINEAR;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.minLod = -1000;
        info.maxLod = 1000;
        info.maxAnisotropy = 1.0f;
        VkResult err = vkCreateSampler(device, &info, nullptr, &sampler);
        check_vk_result(err);
    }

    // Create Descriptor Set
    descriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Image::setData(const void *data) {
    VkDevice device = Application::getDevice();
    size_t uploadSize = width * height * bytesPerPixel(format);

    VkResult err;

    if (!stagingBuffer) {
        // create upload buffer;
        {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = uploadSize;
            bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            err = vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer);
            check_vk_result(err);

            VkMemoryRequirements req;
            vkGetBufferMemoryRequirements(device, stagingBuffer, &req);
            alignedSize = req.size;

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = req.size;
            allocInfo.memoryTypeIndex = getVulkanMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
            err = vkAllocateMemory(device, &allocInfo, nullptr, &stagingBufferMemory);
            check_vk_result(err);
            err = vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);
            check_vk_result(err);
        }
    }

    // Upload to Buffer
    {
        char *map = NULL;
        err = vkMapMemory(device, stagingBufferMemory, 0, alignedSize, 0, (void**)&map);
        check_vk_result(err);
        memcpy(map, data, uploadSize);
        VkMappedMemoryRange range[1] = {};
        range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range[0].memory = stagingBufferMemory;
        range[0].size = alignedSize;
        err = vkFlushMappedMemoryRanges(device, 1, range);
        check_vk_result(err);
        vkUnmapMemory(device, stagingBufferMemory);
    }

    // Copy to Image
    {
        VkCommandBuffer commandBuffer = Application::getCommandBuffer(true);

        VkImageMemoryBarrier copyBarrier{};
        copyBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        copyBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        copyBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        copyBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copyBarrier.image = image;
        copyBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyBarrier.subresourceRange.levelCount = 1;
        copyBarrier.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &copyBarrier);

        VkBufferImageCopy region{};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width = width;
        region.imageExtent.height = height;
        region.imageExtent.depth = 1;
        vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        VkImageMemoryBarrier useBarrier{};
        useBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        useBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        useBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        useBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        useBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        useBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        useBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        useBarrier.image = image;
        useBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        useBarrier.subresourceRange.levelCount = 1;
        useBarrier.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &useBarrier);
    
        Application::flushCommandBuffer(commandBuffer);
    }
}

void Image::release() {
    Application::submitResourceFree([sampler = this->sampler, imageView = this->imageView, image = this->image, memory = this->memory, stagingBuffer = this->stagingBuffer, stagingBufferMemory = this->stagingBufferMemory](){
        VkDevice device = Application::getDevice();

        vkDestroySampler(device, sampler, nullptr);
        vkDestroyImageView(device, imageView, nullptr);
        vkDestroyImage(device, image, nullptr);
        vkFreeMemory(device, memory, nullptr);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    });

    sampler = nullptr;
    imageView = nullptr;
    image = nullptr;
    memory = nullptr;
    stagingBuffer = nullptr;
    stagingBufferMemory = nullptr;
}

Image::Image(std::string_view path) : filepath(path) {
    int w, h, channels;
    uint8_t *data = nullptr;

    if (stbi_is_hdr(filepath.c_str())) {
        data = (uint8_t*)stbi_loadf(filepath.c_str(), &w, &h, &channels, 4);
        format = ImageFormat::RGBA32F;
    }
    else {
        data = stbi_load(filepath.c_str(), &w, &h, &channels, 4);
        format = ImageFormat::RGBA;
    }

    this->width = w;
    this->height = h;

    allocateMemory(width * height * bytesPerPixel(format));

    stbi_image_free(data);
}

Image::Image(uint32_t width, uint32_t height, ImageFormat format, const void *data) 
    : width(width), height(height), format(format) {
    allocateMemory(width * height * bytesPerPixel(format));
    if (data) {
        setData(data);
    }
}

Image::~Image() {
    release();
}

void Image::resize(uint32_t newWidth, uint32_t newHeight) {
    if (image && width == newWidth && height == newHeight) {
        return;
    }

    width = newWidth;
    height = newHeight;

    release();
    allocateMemory(width * height * bytesPerPixel(format));
}