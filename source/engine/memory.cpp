#include "memory.h"

u32 DEUtil::FindMemoryTypeIndex(vk::PhysicalDevice device, u32 supportedMemIndices,
                                vk::MemoryPropertyFlags requestedProperties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = device.getMemoryProperties();

    for(u32 i = 0; i < memProperties.memoryTypeCount; i++)
    {
        bool supported  = static_cast<bool>(supportedMemIndices & BIT(i));
        bool sufficient = (memProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties;

        if(supported && sufficient)
        {
            return i;
        }
    }

    return -1;
}

void DEUtil::AllocateBufferMemory(Buffer &buff, const BufferInput &buffIn)
{
    vk::MemoryRequirements memReq = buffIn.logicalDevice.getBufferMemoryRequirements(buff.buffer);

    vk::MemoryAllocateInfo allocInfo;
    allocInfo.allocationSize = memReq.size;
    // clang-format off
    allocInfo.memoryTypeIndex = FindMemoryTypeIndex(
        buffIn.physicalDevice, memReq.memoryTypeBits,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );
    // clang-format on

    buff.memory = buffIn.logicalDevice.allocateMemory(allocInfo);
    buffIn.logicalDevice.bindBufferMemory(buff.buffer, buff.memory, 0);
}

DEUtil::Buffer DEUtil::CreateBuffer(BufferInput buffIn)
{
    vk::BufferCreateInfo buffInfo;
    buffInfo.flags       = vk::BufferCreateFlags();
    buffInfo.size        = buffIn.size;
    buffInfo.usage       = buffIn.usage;
    buffInfo.sharingMode = vk::SharingMode::eConcurrent;

    // NOTE: need to specify queue families...

    Buffer buffer;
    buffer.buffer = buffIn.logicalDevice.createBuffer(buffInfo);
    AllocateBufferMemory(buffer, buffIn);

    return buffer;
}