#pragma once
#include "Iris/Entity/Component.hpp"

namespace Iris {
    class Material final : public Component {
    public:
        Material(size_t parentId, const std::shared_ptr<Scene>& scene) : Component(parentId, scene) {}

        Material(size_t parentId, const std::shared_ptr<Scene>& scene, std::string_view texture) :
                Component(parentId, scene), m_Texture(texture) {}

        [[nodiscard]] const std::string& getTexture() const { return m_Texture; }

    private:
        glm::vec3 m_Ambient{};
        glm::vec3 m_Diffuse{};
        glm::vec3 m_Specular{};
        std::string m_Texture;
    };
}
