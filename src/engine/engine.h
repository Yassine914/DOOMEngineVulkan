#pragma once

#include <DEngine.h>
#include "../core/window.h"

#pragma region Swapchain and Pipeline

struct SwapChainFrame
{
    vk::Image image;
    vk::ImageView imageView;
    vk::Framebuffer frameBuffer;
    vk::CommandBuffer commandBuffer;
    vk::Semaphore imageAvailable, renderFinished;
    vk::Fence inFlight;
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
    i32 width, height;

    //_____ VK VARS _____
    vk::Instance vkInstance{nullptr};

    // debug callback and dynamic loader
    vk::DebugUtilsMessengerEXT debugMessenger{nullptr};
    vk::DispatchLoaderDynamic dldi;
    vk::SurfaceKHR surface;

    // device
    vk::PhysicalDevice physicalDevice{nullptr};
    vk::Device device{nullptr};
    vk::Queue graphicsQueue{nullptr};
    vk::Queue presentQueue{nullptr};

    // present (swapchain)
    SwapChainBundle swapchain;

    // pipeline
    GraphicsPipelineBundle pipeline;

    // commands
    vk::CommandPool commandPool;
    vk::CommandBuffer mainCommandBuffer;

    // synchronization
    i32 maxFramesInFlight, frameNum;

    private:
    //_____ VK SPECIFIC _____
    void MakeVKInstance(std::string name);
    void MakeVKDebugMessenger();

    // device
    void ChooseVKPhysicalDevice();
    void MakeVKLogicalDevice(vk::PhysicalDevice device);
    void MakeVKQueues(vk::Device device, vk::PhysicalDevice phyDevice);

    // present
    void MakeVKSwapChain(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);

    // pipeline
    void MakeVKGraphicsPipeline();

    // finalizing initialization
    void InitializeVKDrawing();

    // commands
    void RecordVKDrawCommands(vk::CommandBuffer commandBuffer, u32 imageIdx);

    public:
    // constructor and destructor
    Engine(i32 width, i32 height, Window *window);

    void Render();

    ~Engine();
};