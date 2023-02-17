#include "Light.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <imgui.h>

namespace Iris {
    Light::Light(const size_t& parentId, const std::shared_ptr<Scene>& scene) : Component(parentId, scene) {}

    Light::Light(const size_t& parentId, const std::shared_ptr<Scene>& scene, glm::vec3 color, LightType type)
            : Light(parentId, scene) {
        this->color = color;
        this->type = type;
    }

    void Light::RenderUI() {
        ImGui::ColorEdit3("Color", glm::value_ptr(color));
        static const char* items[]{ "Point", "Directional", "Spot" };
        int selected = static_cast<int>(type);

        if (ImGui::Combo("MyCombo", &selected, items, IM_ARRAYSIZE(items))) {
            type = static_cast<LightType>(selected);
        }
    }
}
