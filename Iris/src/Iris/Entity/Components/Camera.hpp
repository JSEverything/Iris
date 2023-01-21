#pragma once
#include "Iris/Entity/Component.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Iris {
    class Camera final : public Component {
    public:
        Camera(size_t parentId, const std::shared_ptr<Scene>& scene, float fov = 90.f, float aspect = 1600.f / 900.f,
               float nearClip = 0.1f, float farClip = 1000.f);

        void SetViewportSize(glm::vec2 size);
        void Update(float dt) override;

        [[nodiscard]] glm::mat4 GetProjectionMatrix() const;
        [[nodiscard]] glm::mat4 GetViewMatrix() const;
    private:
        void UpdateProjection();
        void UpdateView();
        [[nodiscard]] glm::vec2 PanSpeed() const;
        [[nodiscard]] float RotationSpeed() const;
        [[nodiscard]] float ZoomSpeed() const;

        void MousePan(glm::vec2 delta);
        void MouseRotate(glm::vec2 delta);
        void MouseZoom(float delta);

        [[nodiscard]] glm::vec3 GetUpDirection() const;
        [[nodiscard]] glm::vec3 GetForwardDirection() const;
        [[nodiscard]] glm::vec3 GetRightDirection() const;
        [[nodiscard]] glm::vec3 CalculatePosition() const;
        [[nodiscard]] glm::quat GetOrientation() const;

    private:
        glm::mat4 m_Projection{};
        glm::mat4 m_View{};

        float m_FOV;
        float m_AspectRatio;
        glm::vec2 m_ViewportSize{ 1600.f, 900.f };
        float m_NearClip;
        float m_FarClip;

        glm::vec3 m_FocalPoint = { 0.f, 0.f, 0.f };
        float m_Pitch = 0.f, m_Yaw = 0.f;
        float m_Distance = 10.f;
        glm::vec2 m_InitialMousePos{ 0.f };
    };
}
