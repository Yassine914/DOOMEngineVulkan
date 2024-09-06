#pragma once

#include <DEngine.h>
#include "../engine/memory.h"

class TriangleMesh
{
    private:
    vk::Device device;

    public:
    DEUtil::Buffer vertexBuffer;

    private:
    public:
    TriangleMesh(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice);

    ~TriangleMesh();
};
