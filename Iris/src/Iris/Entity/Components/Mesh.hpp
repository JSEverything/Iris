#pragma once
#include "Iris/Entity/Component.hpp"
#include "Iris/Renderer/Vertex.hpp"

namespace Iris {
    class Mesh final : public Component {
    public:
        Mesh(size_t parentId, const std::shared_ptr<Scene>& scene) : Component(parentId, scene) {}
        Mesh(size_t parentId, const std::shared_ptr<Scene>& scene, std::vector<Vertex> vertices,  std::vector<uint32_t> indices);
        Mesh(size_t parentId, const std::shared_ptr<Scene>& scene, std::string_view path);

        void SetMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

        [[nodiscard]] const std::vector<Vertex>& GetVertices() const;
        [[nodiscard]] const std::vector<uint32_t>& GetIndices() const;
        [[nodiscard]] glm::mat4 GetModelMatrix() const;
    private:
        std::vector<Vertex> m_Vertices{};
        std::vector<uint32_t> m_Indices{};
    };
}
