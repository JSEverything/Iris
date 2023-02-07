#include "Component.hpp"
#include "Iris/Scene/Scene.hpp"

namespace Iris {
    Entity& Component::GetParent() const {
        return m_Scene->GetEntity(m_ParentId);
    }
}
