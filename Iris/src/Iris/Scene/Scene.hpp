#pragma once
#include "Iris/Entity/Entity.hpp"
#include "Iris/Util/EventEmitter.hpp"

namespace Iris {
    struct ObjectAdd final : public EventHandler<size_t> {
    };
    struct ObjectRemove final : public EventHandler<size_t> {
    };

    class Scene final : public EventEmitter<
            ObjectAdd,
            ObjectRemove
    > {
    public:
        Scene() = default;
        Entity& CreateObject();
        void AddObject(Entity& entity);
        std::vector<Entity>& GetObjects();
        void Update(float dt);
        Entity& GetEntity(size_t id) { return m_Entities[id]; };
        void SetThis(const std::shared_ptr<Scene>& ptr) {m_This = ptr;};
    private:
        std::vector<Entity> m_Entities{};
        std::weak_ptr<Scene> m_This;
    };
}
