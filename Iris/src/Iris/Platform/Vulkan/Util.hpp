#pragma once
#include <vulkan/vulkan.hpp>
#include <Iris/Renderer/Vertex.hpp>

namespace Iris::Vulkan {
    void VkCheck(VkResult result, std::string_view msg);

    void VkCheck(vk::Result result, std::string_view msg);

    uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits,
                            vk::MemoryPropertyFlags requirementsMask);

    vk::Format attributeToFormat(Iris::Vertex::Attribute attribute);

    std::vector<vk::VertexInputAttributeDescription>
    parseVertexDescription(const std::vector<Iris::Vertex::Attribute>& attributes, uint32_t binding);
}