#pragma once
#include "Iris/Entity/Component.hpp"

namespace Iris {
    class Transform final : public Component {
    public:
        Transform(size_t parentId, const std::shared_ptr<Scene>& scene) : Component(parentId, scene) {}

        [[nodiscard]] const glm::vec3& GetTranslation() const;
        [[nodiscard]] const glm::vec3& GetRotation() const;

        void SetTranslation(const glm::vec3& mTranslation);
        void SetRotation(const glm::vec3& mRotation);

        void Move(const glm::vec3& offset);
        void Rotate(const glm::vec3& offset);

        void Reset();
    private:
        glm::vec3 m_Translation{};
        glm::vec3 m_Rotation{};
    };
}
