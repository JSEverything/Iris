#pragma once
#include "Iris/Entity/Component.hpp"
#include "Iris/Renderer/MVP.hpp"

namespace Iris {
    class Camera final : public Component {
    public:
        Camera(size_t parentId, const std::shared_ptr<Scene>& scene);

        void Update(float dt) override;

        [[nodiscard]] glm::mat4 GetCameraMatrix() const;
        [[nodiscard]] glm::mat4 GetViewMatrix() const;
    private:
        float m_FOV{90.f};
        glm::vec3 m_InputAxis{};
        glm::vec3 m_Velocity{};
        glm::mat4 m_Projection{};
        glm::mat4 m_View{};
    };
}
