#include "Camera.hpp"
#include "Transform.hpp"
#include "Iris/Entity/Entity.hpp"
#include "Iris/Util/Input.hpp"
#include "Iris/Scene/Scene.hpp"

namespace Iris {
    Camera::Camera(size_t parentId, const std::shared_ptr<Scene>& scene, float fov, float aspect, float nearClip,
                   float farClip)
            : Component(parentId, scene), m_FOV(fov), m_AspectRatio(aspect), m_NearClip(nearClip), m_FarClip(farClip) {
        UpdateView();
        UpdateProjection();
    }

    void Camera::SetViewportSize(glm::vec2 size) {
        m_ViewportSize = size;
        UpdateProjection();
    }

    void Camera::Update(float dt) {
        if (Input::IsKeyPressed(GLFW_KEY_LEFT_ALT)) {
            const glm::vec2& mouse = Input::GetMousePos();
            glm::vec2 delta = (mouse - m_InitialMousePos);
            m_InitialMousePos = mouse;

            if (Input::IsKeyPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
                MousePan(delta * 0.009f);
            } else if (Input::IsKeyPressed(GLFW_MOUSE_BUTTON_1)) {
                MouseRotate(delta * 0.003f);
            } else if (Input::IsKeyPressed(GLFW_MOUSE_BUTTON_2)) {
                MouseZoom(delta.y * 0.003f);
            }
        }

        Input::Get().on<MouseScroll>([this](glm::vec2 offset) {
            float delta = offset.y * 0.0001f;
            MouseZoom(delta);
            UpdateView();
        });

        UpdateView();
    }

    glm::mat4 Camera::GetProjectionMatrix() const {
        return m_Projection;
    }

    glm::mat4 Camera::GetViewMatrix() const {
        return m_View;
    }

    void Camera::UpdateProjection() {
        m_AspectRatio = m_ViewportSize.x / m_ViewportSize.y;
        m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
    }

    void Camera::UpdateView() {
        if (m_Scene) {
            auto& transform = m_Scene->GetEntity(m_ParentId).GetTransform();
            transform.SetTranslation(GetPosition());
        }

        glm::quat orientation = GetOrientation();
        m_View = glm::translate(glm::mat4(1.f), GetPosition()) * glm::toMat4(orientation);
        m_View = glm::inverse(m_View);
    }

    glm::vec2 Camera::PanSpeed() const {
        float x = glm::min(m_ViewportSize.x / 1000.f, 2.4f);
        float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;
        float y = glm::min(m_ViewportSize.y / 1000.f, 2.4f);
        float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;
        return { xFactor, yFactor };
    }

    float Camera::RotationSpeed() const {
        return 0.8f;
    }

    float Camera::ZoomSpeed() const {
        float distance = glm::max(m_Distance * 0.2f, 3.f);
        float speed = glm::min(distance * distance, 100.f);
        return speed;
    }

    void Camera::MousePan(glm::vec2 delta) {
        auto speed = PanSpeed();
        m_FocalPoint += -GetRightDirection() * delta.x * speed.x * m_Distance;
        m_FocalPoint += GetUpDirection() * delta.y * speed.y * m_Distance;
    }

    void Camera::MouseRotate(glm::vec2 delta) {
        m_Yaw += glm::sign(GetUpDirection().y) * delta.x * RotationSpeed();
        m_Pitch += delta.y * RotationSpeed();
    }

    void Camera::MouseZoom(float delta) {
        m_Distance -= delta * ZoomSpeed();
        if (m_Distance < 1.0f) {
            m_Distance = 1.0f;
        }
    }

    glm::vec3 Camera::GetUpDirection() const {
        return glm::rotate(GetOrientation(), { 0.f, 1.f, 0.f });
    }

    glm::vec3 Camera::GetForwardDirection() const {
        return glm::rotate(GetOrientation(), { 0.f, 0.f, -1.f });
    }

    glm::vec3 Camera::GetRightDirection() const {
        return glm::rotate(GetOrientation(), { 1.f, 0.f, 0.f });
    }

    glm::vec3 Camera::GetPosition() const {
        return m_FocalPoint - GetForwardDirection() * m_Distance;
    }

    glm::quat Camera::GetOrientation() const {
        return { glm::vec3(-m_Pitch, -m_Yaw, 0.f) };
    }
}
