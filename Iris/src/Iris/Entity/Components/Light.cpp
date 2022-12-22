#include "Light.hpp"

namespace Iris {
    Light::Light(const size_t& parentId, const std::shared_ptr<Scene>& scene) : Component(parentId, scene) {}
}
