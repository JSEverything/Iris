#include "Vertex.hpp"

namespace Iris {
    std::vector<Vertex::Attribute> Vertex::GetDescription() {
        return { { Attribute::Type::Float, sizeof(Vertex::position), offsetof(Vertex, position) },
                 { Attribute::Type::Float, sizeof(Vertex::color),    offsetof(Vertex, color) },
                 { Attribute::Type::Float, sizeof(Vertex::normal),   offsetof(Vertex, normal) },
                 { Attribute::Type::Float, sizeof(Vertex::uv),       offsetof(Vertex, uv) } };
    }
}
