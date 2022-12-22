#include "Entity.hpp"

#include <utility>
#include "Components/Camera.hpp"

namespace Iris {
    void Entity::Update(float dt) {
        UpdateT<Camera>(dt);
    }

    Entity::Entity(size_t id, std::shared_ptr<Scene> scene) : m_Id(id), m_Scene(std::move(scene)), m_Transform(m_Id, m_Scene) {}
}
