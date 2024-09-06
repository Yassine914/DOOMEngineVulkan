#include "triangle.h"

TriangleMesh::TriangleMesh(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice)
{
    this->device = logicalDevice;

    // clang-format off
    std::vector<f32> vertices = {
        0.0f, -0.05f, 1.0f, 0.0f, 0.0f,
        0.05f, 0.05f, 0.0f, 1.0f, 0.0f,
        -0.05f, 0.05f, 0.0f, 0.0f, 1.0f
    };

    // clang-format on

    DEUtil::BufferInput buffIn;
    buffIn.logicalDevice  = logicalDevice;
    buffIn.physicalDevice = physicalDevice;
    buffIn.size           = vertices.size() * sizeof(f32);
    buffIn.usage          = vk::BufferUsageFlagBits::eVertexBuffer;

    vertexBuffer = DEUtil::CreateBuffer(buffIn);

    void *memLoc = logicalDevice.mapMemory(vertexBuffer.memory, 0, buffIn.size);
    memcpy(memLoc, vertices.data(), buffIn.size);

    logicalDevice.unmapMemory(vertexBuffer.memory);
}

TriangleMesh::~TriangleMesh()
{
    device.destroyBuffer(vertexBuffer.buffer);
    device.freeMemory(vertexBuffer.memory);
}
