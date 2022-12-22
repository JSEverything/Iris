#pragma once

namespace Iris {
    struct MVPC {
        glm::mat4x4 model;
        glm::mat4x4 view;
        glm::mat4x4 projection;
        glm::mat4x4 clip;
    };
}
