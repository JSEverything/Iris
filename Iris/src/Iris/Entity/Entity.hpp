#pragma once
#include "Iris/Entity/Component.hpp"
#include "Iris/Entity/ComponentStore.hpp"
#include "Iris/Entity/Components/Transform.hpp"
#include "Iris/Entity/Components/Camera.hpp"
#include "Iris/Entity/Components/Material.hpp"
#include "Iris/Entity/Components/Mesh.hpp"
#include "Iris/Entity/Components/Light.hpp"

namespace Iris {
    class Scene;

    class Entity final {
    public:
        Entity(size_t id, std::shared_ptr<Scene> scene);

        void Update(float dt);

        Transform& GetTransform() { return m_Transform; };

        template <class T>
        std::vector<T>& GetComponents() {
            return m_Components.template GetComponents<T>();
        }

        template <class T>
        T& GetComponent() {
            return m_Components.template GetComponents<T>()[0];
        }

        template <class T, class... Ts>
        Entity& AddComponent(Ts... args) {
            m_Components.template GetComponents<T>().emplace_back(m_Id, m_Scene, args...);
            return *this;
        }

        [[nodiscard]] size_t GetId() const { return m_Id; };
        void RenderUI();
    private:
        template <class T>
        requires std::is_base_of_v<Component, T>
        void UpdateT(float dt) {
            for (T& item: GetComponents<T>()) {
                item.Update(dt);
            }
        }

    private:
        size_t m_Id;
        std::shared_ptr<Scene> m_Scene;
        Transform m_Transform;

        ComponentStore<
                Camera,
                Material,
                Mesh,
                Light
        > m_Components;
    };
}
