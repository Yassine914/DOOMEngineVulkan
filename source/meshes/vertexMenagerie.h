#pragma once

#include <DEngine.h>
#include "../engine/memory.h"

enum class MeshType
{
    TRIANGLE,
    QUAD,
    POLYGON
};

struct VertexData
{
    i32 offset;
    i32 size;
};

class VertexMenagerie
{
    private:
    i32 offset;
    std::vector<f32> lump;

    vk::Device device;

    public:
    DEUtil::Buffer vertexBuffer;
    std::unordered_map<MeshType, VertexData> vertexAttribData;

    public:
    VertexMenagerie();

    void Consume(MeshType type, std::vector<f32> vertexData);
    void Finalize(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice);

    ~VertexMenagerie();
};