#pragma once

#include <DEngine.h>

namespace DEUtil {

struct BufferInput
{
    usize size;
    vk::BufferUsageFlags usage;
    vk::Device logicalDevice;
    vk::PhysicalDevice physicalDevice;
};

struct Buffer
{
    vk::Buffer buffer;
    vk::DeviceMemory memory;
};

u32 FindMemoryTypeIndex(vk::PhysicalDevice device, u32 supportedMemIndices,
                        vk::MemoryPropertyFlags requestedProperties);

void AllocateBufferMemory(Buffer &buff, const BufferInput &buffIn);

Buffer CreateBuffer(BufferInput buffIn);

} // namespace DEUtil