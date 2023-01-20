#pragma once
#include "glm/vec3.hpp"

namespace Iris {
    class Vertex {
    public:
        glm::vec4 position;
        glm::vec4 color{};
        glm::vec4 normal{};

        glm::vec2 uv{};
    };
}
