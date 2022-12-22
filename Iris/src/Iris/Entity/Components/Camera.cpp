#include "Camera.hpp"
#include "Transform.hpp"
#include "Iris/Entity/Entity.hpp"
#include "Iris/Util/EventProxy.hpp"
#include "Iris/Scene/Scene.hpp"

namespace Iris {
    Camera::Camera(size_t parentId, const std::shared_ptr<Scene>& scene) : Component(parentId, scene) {
        float aspect = 1600.f / 900.f;

        m_View = glm::mat4(1.f);
        m_Projection = glm::perspective(glm::radians(m_FOV), aspect, 0.1f, 100.0f);

        auto& input = EventProxy::Get();
        float step = 0.1f;
        input.on<Key>([=, this](int key, KeyMods mods){
            auto& transform = m_Scene->GetEntity(m_ParentId).GetTransform();
            switch (key) {
                case GLFW_KEY_W:
                    transform.Move({0.f, 0.f, step});
                    break;
                case GLFW_KEY_S:
                    transform.Move({0.f, 0.f, -step});
                    break;
                case GLFW_KEY_A:
                    transform.Move({step, 0.f, 0.f});
                    break;
                case GLFW_KEY_D:
                    transform.Move({-step, 0.f, 0.f});
                    break;
                case GLFW_KEY_SPACE:
                    transform.Move({0.f, -step, 0.f});
                    break;
                case GLFW_KEY_LEFT_SHIFT:
                    transform.Move({0.f, step, 0.f});
                    break;
                default:
                    break;
            }
        });
    }

    void Camera::Update(float dt) {
        auto& transform = m_Scene->GetEntity(m_ParentId).GetTransform();

        glm::mat4 yaw_rot = glm::rotate(glm::mat4{1.f}, transform.GetRotation().x, {0.f, -1.f, 0.f});
        glm::mat4 pitch_rot = glm::rotate(glm::mat4{yaw_rot}, transform.GetRotation().y, {-1.f, 0.f, 0.f});

        m_View = glm::translate(glm::mat4(1.f), transform.GetTranslation()) * pitch_rot;
    }

    glm::mat4 Camera::GetCameraMatrix() const {
        return m_Projection * m_View;
    }

    glm::mat4 Camera::GetViewMatrix() const {
        return m_View;
    }
}
