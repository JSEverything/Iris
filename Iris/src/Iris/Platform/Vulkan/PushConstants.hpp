#pragma once
#include <glm/glm.hpp>

namespace Iris::Vulkan {
    struct PushConstants {
        glm::mat4 modelMat;
        uint32_t objectID;
        //uint32_t materialID;
    };
}