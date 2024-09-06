#pragma once

#include "init.h"
#include "device.h"

class Engine
{
    private:
    // vulkan instance
    vk::Instance vkInstance;

    // debug messenger
    vk::DebugUtilsMessengerEXT debugMessenger;
    vk::DispatchLoaderDynamic dispatchLoader;

    // devices
    vk::PhysicalDevice phyDevice;
    vk::Device device;

    // queues
    DE::VK::QueueFamilyIndices indices;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;

    public:
    Engine();

    ~Engine();
};