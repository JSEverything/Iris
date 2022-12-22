#pragma once
#include "Iris/Entity/Component.hpp"

namespace Iris {
    class Material final : public Component {
    public:
        Material(size_t parentId, const std::shared_ptr<Scene>& scene) : Component(parentId, scene) {}

    private:
        glm::vec3 m_Ambient;
        glm::vec3 m_Diffuse;
        glm::vec3 m_Specular;
        float shininess;
    };
}
