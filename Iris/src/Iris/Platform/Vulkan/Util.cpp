#include "Util.hpp"
#include "Iris/Core/Log.hpp"

namespace Iris::Vulkan {
    void VkCheck(VkResult result, std::string_view msg) {
        if (result == VK_SUCCESS) return;
        Iris::Log::Core::Error("VkResult Error: {}\n", msg);
        std::exit(1);
    }

    void VkCheck(vk::Result result, std::string_view msg) {
        if (result == vk::Result::eSuccess) return;
        Iris::Log::Core::Error("vk::Result Error: {}\n", msg);
        std::exit(1);
    }

    uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits,
                            vk::MemoryPropertyFlags requirementsMask) {
        uint32_t typeIndex = -1;
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            if ((typeBits & 1) &&
                ((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)) {
                typeIndex = i;
                break;
            }
            typeBits >>= 1;
        }
        assert(typeIndex != -1);
        return typeIndex;
    }

    vk::Format attributeToFormat(Iris::Vertex::Attribute attribute) {
        switch (attribute.type) {
            case Iris::Vertex::Attribute::Type::Float:
                switch (attribute.size) {
                    case 4:
                        return vk::Format::eR32Sfloat;
                    case 8:
                        return vk::Format::eR32G32Sfloat;
                    case 12:
                        return vk::Format::eR32G32B32Sfloat;
                    case 16:
                        return vk::Format::eR32G32B32A32Sfloat;
                    default:
                        Iris::Log::Core::Critical("Missing Vulkan mapping for vertex attribute! Type: {}, Size: {}",
                                                  static_cast<uint32_t>(attribute.type), attribute.size);
                        std::exit(1);
                }
            case Iris::Vertex::Attribute::Type::UInt:
                switch (attribute.size) {
                    case 4:
                        return vk::Format::eR32Uint;
                    case 8:
                        return vk::Format::eR32G32Uint;
                    case 12:
                        return vk::Format::eR32G32B32Uint;
                    case 16:
                        return vk::Format::eR32G32B32A32Uint;
                    default:
                        Iris::Log::Core::Critical("Missing Vulkan mapping for vertex attribute!");
                        std::exit(1);
                }
            case Iris::Vertex::Attribute::Type::Int:
                switch (attribute.size) {
                    case 4:
                        return vk::Format::eR32Sint;
                    case 8:
                        return vk::Format::eR32G32Sint;
                    case 12:
                        return vk::Format::eR32G32B32Sint;
                    case 16:
                        return vk::Format::eR32G32B32A32Sint;
                    default:
                        Iris::Log::Core::Critical("Missing Vulkan mapping for vertex attribute!");
                        std::exit(1);
                }
        }
    }

    std::vector<vk::VertexInputAttributeDescription>
    parseVertexDescription(const std::vector<Iris::Vertex::Attribute>& attributes, uint32_t binding) {
        std::vector<vk::VertexInputAttributeDescription> out;

        for (uint32_t location = 0; location < attributes.size(); ++location) {
            auto format = attributeToFormat(attributes[location]);
            out.emplace_back(location, binding, format, attributes[location].offset);
        }

        return out;
    }
}
