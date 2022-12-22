#include "Transform.hpp"

namespace Iris {
    const glm::vec3& Transform::GetTranslation() const {
        return m_Translation;
    }

    const glm::vec3& Transform::GetRotation() const {
        return m_Rotation;
    }

    void Transform::SetTranslation(const glm::vec3& translation) {
        m_Translation = translation;
    }

    void Transform::SetRotation(const glm::vec3& rotation) {
        m_Rotation = rotation;
    }

    void Transform::Move(const glm::vec3& offset) {
        m_Translation += offset;
    }

    void Transform::Rotate(const glm::vec3& offset) {
        m_Rotation += offset;
    }

    void Transform::Reset() {
        m_Translation = { 0.f, 0.f, 0.f };
        m_Rotation = { 0.f, 0.f, 0.f };
    }
}
