#pragma once
#include <string>

#include <vulkan/vulkan.h>

enum class ImageFormat {
    NONE = 0,
    RGBA,
    RGBA32F,
};

class Image {
    uint32_t width = 0, height = 0;
    ImageFormat format = ImageFormat::NONE;
    std::string filepath;

    VkDescriptorSet descriptorSet = nullptr;
    
    VkImage image = nullptr;
    VkImageView imageView = nullptr;
    VkDeviceMemory memory = nullptr;
    VkSampler sampler = nullptr;

    size_t alignedSize = 0;
    VkBuffer stagingBuffer = nullptr;
    VkDeviceMemory stagingBufferMemory = nullptr;


    void allocateMemory(uint64_t size);
    void release();
public:
    Image(std::string_view path);
    Image(uint32_t width, uint32_t height, ImageFormat format, const void *data = nullptr);
    ~Image();


    VkDescriptorSet getDescriptorSet() const { return descriptorSet; }
    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }

    void setData(const void *data);
    void resize(uint32_t newWidth, uint32_t newHeight);
};