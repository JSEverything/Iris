#pragma once
#include "Iris/Entity/Component.hpp"

namespace Iris {
    class Light : public Component {
    public:
        Light(const size_t& parentId, const std::shared_ptr<Scene>& scene);
    };
}
