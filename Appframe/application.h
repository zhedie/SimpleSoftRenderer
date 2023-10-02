#pragma once
#include "Layer.h"

#include <string>
#include <functional>
#include <vector>
#include <memory>

#include <vulkan/vulkan.h>

void check_vk_result(VkResult err);

struct ApplicationSpecification {
    uint32_t width = 1600;
    uint32_t height = 900;
    std::string name;
};

struct GLFWwindow;

class Application {
    ApplicationSpecification appSpec;
    GLFWwindow *windowHandle = nullptr;
    bool running = false;

    float timeStep = 0.0f;
    float frameTime = 0.0f;
    float lastFrameTime = 0.0f;

    std::function<void()> menubarCallback;
    std::vector<std::shared_ptr<Layer>> layerStack;

    static Application *instance;

private:
    void init();
    void shutdown();
public:
    Application(const ApplicationSpecification &_appSpec = ApplicationSpecification{});
    ~Application();

    static Application* get();

    float getTime();
    GLFWwindow *getWindowHandle() const { return windowHandle; }

    void run();
    void stop();

    template<typename T>
    void pushLayer() {
        static_assert(std::is_base_of<Layer, T>::value, "Pushed type is not subclass of Layer!");
        layerStack.emplace_back(std::make_shared<T>())->onAttach();
    }
    template<typename T>
    void pushLayer(const std::shared_ptr<T> &layer) {
        static_assert(std::is_base_of<Layer, T>::value, "Pushed type is not subclass of Layer!");
        layerStack.emplace_back(layer)->onAttach();
    }
    void pushLayer(const std::shared_ptr<Layer> &layer);
    void setMenubarCallback(const std::function<void()> &_menubarCallback);

    static VkInstance getInstance();
    static VkPhysicalDevice getPhysicalDevice();
    static VkDevice getDevice();

    static VkCommandBuffer getCommandBuffer(bool begin);
    static void flushCommandBuffer(VkCommandBuffer commandBuffer);
    static void submitResourceFree(std::function<void()> &&func);
};

// Implemented by client
Application* createApplication();