#include "Util.hpp"
#include "Iris/Core/Log.hpp"

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