#pragma once
#include "glm/vec3.hpp"

namespace Iris {
    class Vertex {
    public:
        class Attribute;

        glm::vec4 position;
        glm::vec4 color{};
        glm::vec4 normal{};

        glm::vec2 uv{};

        static std::vector<Attribute> GetDescription();

        struct Attribute {
            enum class Type : uint16_t {
                Float,
                Int,
                UInt
            };

            Type type;
            size_t size;
            size_t offset;
        };
    };
}
