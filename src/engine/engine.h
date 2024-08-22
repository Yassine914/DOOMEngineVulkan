#pragma once

#include <DEngine.h>
#include "../core/window.h"

#pragma region Swapchain
struct SwapChainBundle
{
    vk::SwapchainKHR swapchain;
    std::vector<vk::Image> images;
    vk::Format format;
    vk::Extent2D extent;
};

#pragma endregion

class Engine
{
    private:
    Window *window;

    //_____ VK VARS _____
    vk::Instance vkInstance{nullptr};

    // debug callback and dynamic loader
    vk::DebugUtilsMessengerEXT debugMessenger{nullptr};
    vk::DispatchLoaderDynamic dldi;
    vk::SurfaceKHR surface;

    // device
    vk::PhysicalDevice physicalDevice{nullptr};
    vk::Device logicalDevice{nullptr};
    vk::Queue graphicsQueue{nullptr};
    vk::Queue presentQueue{nullptr};

    // present (swapchain)
    SwapChainBundle swapchain;

    public:
    Engine();

    //_____ VK SPECIFIC _____
    void MakeVKInstance(std::string name);
    void MakeVKDebugMessenger();

    // device
    void ChooseVKPhysicalDevice();
    void MakeVKLogicalDevice(vk::PhysicalDevice device);
    void MakeVKQueues(vk::Device device, vk::PhysicalDevice phyDevice);

    // present
    void MakeVKSwapChain(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, i32 width,
                         i32 height);

    //_____ ENGINE SPECIFIC _____
    bool RunEngine()
    {
        window->NewFrame();
        return !window->WindowShouldClose();
    }

    ~Engine();
};