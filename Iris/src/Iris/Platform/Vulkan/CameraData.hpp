#pragma once
#include <glm/glm.hpp>

namespace Iris::Vulkan {
    struct CameraData {
        glm::vec4 position = glm::vec4(1.f);
        glm::vec4 forward = glm::vec4(1.f);
        glm::vec4 up = glm::vec4(1.f);
        glm::vec4 right = glm::vec4(1.f);
        glm::mat4 view = glm::mat4(1.f);
        glm::mat4 projection = glm::mat4(1.f);
        glm::mat4 viewProjection = glm::mat4(1.f); // pre-computed once per frame instead of once per vertex
    };
}