#pragma once
#include "Iris/Entity/Component.hpp"

namespace Iris {
    class Transform final : public Component {
    public:
        Transform(size_t parentId, const std::shared_ptr<Scene>& scene) : Component(parentId, scene) {}

        [[nodiscard]] const glm::vec3& GetTranslation() const;
        [[nodiscard]] const glm::vec3& GetRotation() const;
        [[nodiscard]] const glm::vec3& GetScale() const;
        [[nodiscard]] glm::mat4 GetMatrix() const;

        void SetTranslation(const glm::vec3& translation);
        void SetRotation(const glm::vec3& rotation);
        void SetScale(const glm::vec3& scale);

        void Move(const glm::vec3& offset);
        void Rotate(const glm::vec3& offset);

        void Reset();

        void RenderUI() override;
    private:
        glm::vec3 m_Translation{ 0.f };
        glm::vec3 m_Rotation{ 0.f };
        glm::vec3 m_Scale{ 1.f };
    };
}
