#pragma once
#include "Iris/Entity/Component.hpp"

namespace Iris {
    enum class LightType : uint32_t {
        POINT = 0,
        DIRECTIONAL = 1,
        SPOT = 2
    };

    class Light : public Component {
    public:
        Light(const size_t& parentId, const std::shared_ptr<Scene>& scene);
        Light(const size_t& parentId, const std::shared_ptr<Scene>& scene,
              glm::vec3 color, LightType type = LightType::SPOT);
        glm::vec3 color{ 1.f, 1.f, 1.f };
        LightType type{ LightType::SPOT };

        void RenderUI() override;
    };
}
