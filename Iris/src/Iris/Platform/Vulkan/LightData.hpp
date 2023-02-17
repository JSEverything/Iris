#pragma once
#include <glm/glm.hpp>

namespace Iris::Vulkan {
    struct Light {
        glm::vec4 position;
        glm::vec4 rotation;
        glm::vec4 color;
        glm::uvec4 flags;
    };

    struct LightData {
        uint32_t lightCount;
        char pad[12];
    };
}