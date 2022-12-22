#include "Transform.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Iris {
    const glm::vec3& Transform::GetTranslation() const {
        return m_Translation;
    }

    const glm::vec3& Transform::GetRotation() const {
        return m_Rotation;
    }

    const glm::vec3& Transform::GetScale() const {
        return m_Scale;
    }

    glm::mat4 Transform::GetMatrix() const {
        return glm::scale(
                glm::translate(glm::mat4(1.f), m_Translation) * glm::toMat4(glm::quat(glm::radians(m_Rotation))),
                m_Scale);
    }

    void Transform::SetTranslation(const glm::vec3& translation) {
        m_Translation = translation;
    }

    void Transform::SetRotation(const glm::vec3& rotation) {
        m_Rotation = rotation;
    }

    void Transform::SetScale(const glm::vec3& scale) {
        m_Scale = scale;
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
        m_Scale = { 1.f, 1.f, 1.f };
    }
}
