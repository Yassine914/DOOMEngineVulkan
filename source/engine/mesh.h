#pragma once

#include <DEngine.h>

vk::VertexInputBindingDescription GetPosColorBindingDescription()
{
    vk::VertexInputBindingDescription bindingDesc;
    bindingDesc.binding   = 0;
    bindingDesc.stride    = 5 * sizeof(f32);
    bindingDesc.inputRate = vk::VertexInputRate::eVertex;

    return bindingDesc;
}

std::array<vk::VertexInputAttributeDescription, 2> GetPosColorAttributeDescription()
{
    std::array<vk::VertexInputAttributeDescription, 2> attributes;

    // pos
    attributes[0].binding  = 0;
    attributes[0].location = 0;
    attributes[0].format   = vk::Format::eR32G32Sfloat;
    attributes[0].offset   = 0;

    // col
    attributes[1].binding  = 0;
    attributes[1].location = 1;
    attributes[1].format   = vk::Format::eR32G32B32Sfloat;
    attributes[1].offset   = 2 * sizeof(f32);

    return attributes;
}

class Mesh
{
    private:
    public:
    Mesh();
    ~Mesh();
};