#include "vertexMenagerie.h"

VertexMenagerie::VertexMenagerie()
{
    offset = 0;
}

void VertexMenagerie::Consume(MeshType type, std::vector<f32> vertexData)
{
    for(f32 attrib : vertexData)
    {
        lump.push_back(attrib);
    }

    i32 vertexCount = (i32)vertexData.size() / 5;

    vertexAttribData.insert(std::make_pair(type, VertexData{offset, vertexCount}));

    offset += vertexCount;
}

void VertexMenagerie::Finalize(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice)
{
    this->device = logicalDevice;

    DEUtil::BufferInput buffIn;
    buffIn.logicalDevice  = logicalDevice;
    buffIn.physicalDevice = physicalDevice;
    buffIn.size           = lump.size() * sizeof(f32);
    buffIn.usage          = vk::BufferUsageFlagBits::eVertexBuffer;

    vertexBuffer = DEUtil::CreateBuffer(buffIn);

    void *memLoc = logicalDevice.mapMemory(vertexBuffer.memory, 0, buffIn.size);
    memcpy(memLoc, lump.data(), buffIn.size);

    logicalDevice.unmapMemory(vertexBuffer.memory);
}

VertexMenagerie::~VertexMenagerie()
{
    device.destroyBuffer(vertexBuffer.buffer);
    device.freeMemory(vertexBuffer.memory);
}
