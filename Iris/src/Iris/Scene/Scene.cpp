#include "Scene.hpp"

namespace Iris {
    Entity& Scene::CreateObject() {
        m_Entities.emplace_back(Entity(m_Entities.size(), std::shared_ptr<Scene>(m_This)));
        return m_Entities[m_Entities.size() - 1];
    }

    void Scene::AddObject(Entity& entity) {
        emit<ObjectAdd>(entity.GetId());
    }

    std::vector<Entity>& Scene::GetObjects() {
        return m_Entities;
    }

    void Scene::Update(float dt) {
        for (Entity& entity : m_Entities) {
            entity.Update(dt);
        }
    }
}
