#pragma once

namespace Iris {
    class Entity;

    class Scene;

    class Component {
    public:
        explicit Component(size_t parentId, std::shared_ptr<Scene> scene) : m_ParentId(parentId),
                                                                            m_Scene(std::move(scene)) {}

        [[nodiscard]] Entity& GetParent() const;

        virtual void Update(float dt) {};
    protected:
        size_t m_ParentId;
        std::shared_ptr<Scene> m_Scene;
    };
}
