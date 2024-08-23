#pragma once

#include <DEngine.h>
#include "../core/window.h"

#pragma region Swapchain and Pipeline

struct SwapChainFrame
{
    vk::Image image;
    vk::ImageView imageView;
};

struct SwapChainBundle
{
    vk::SwapchainKHR swapchain;
    std::vector<SwapChainFrame> frames;
    vk::Format format;
    vk::Extent2D extent;
};

struct GraphicsPipelineBundle
{
    vk::PipelineLayout layout;
    vk::RenderPass renderPass;
    vk::Pipeline pipeline;
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

    // pipeline
    GraphicsPipelineBundle pipeline;

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

    // pipeline
    void MakeVKGraphicsPipeline();

    //_____ ENGINE SPECIFIC _____
    bool RunEngine()
    {
        window->NewFrame();
        return !window->WindowShouldClose();
    }

    ~Engine();
};