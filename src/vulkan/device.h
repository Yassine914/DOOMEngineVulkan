#pragma once

#include <DEngine.h>

namespace DE::VK {

struct QueueFamilyIndices
{
    std::optional<u32> graphicsFamily;

    inline bool IsComplete() { return graphicsFamily.has_value(); }
};

vk::PhysicalDevice ChoosePhysicalDevice(const vk::Instance &instance);

struct DeviceProperties
{
    vk::Device device;
    QueueFamilyIndices indices;
};

DeviceProperties MakeLogicalDevice(vk::PhysicalDevice phyDevice);

vk::Queue GetDeviceQueue(const vk::Device &device, const i32 qFamilyIndex, const i32 qIndex);

} // namespace DE::VK