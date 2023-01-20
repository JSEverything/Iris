#include "Mesh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Iris/Scene/Scene.hpp"

namespace Iris {

    void Mesh::SetMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        m_Vertices = vertices;
        m_Indices = indices;
    }

    Mesh::Mesh(size_t parentId, const std::shared_ptr<Scene>& scene, std::vector<Vertex> vertices,
               std::vector<uint32_t> indices)
            : Component(parentId, scene), m_Vertices(std::move(vertices)), m_Indices(std::move(indices)) {}

    Mesh::Mesh(size_t parentId, const std::shared_ptr<Scene>& scene, std::string_view path)
            : Component(parentId, scene) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string err;
        bool status = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.data());
        if (!warn.empty()) {
            Log::Core::Warn("Loading obj: {}", warn);
        }

        if (!err.empty()) {
            Log::Core::Error("Loading obj: {}", err);
        }

        if (!status) {
            Log::Core::Critical("Failed to load obj");
            std::exit(-1);
        }
        m_Vertices.reserve(attrib.vertices.size() / 3);
        for (size_t i = 0; i < attrib.vertices.size() / 3; ++i) {
            float px = attrib.vertices[i * 3];
            float py = attrib.vertices[i * 3 + 1];
            float pz = attrib.vertices[i * 3 + 2];

            float cr = attrib.colors[i * 3];
            float cg = attrib.colors[i * 3 + 1];
            float cb = attrib.colors[i * 3 + 2];

            m_Vertices.emplace_back(Vertex{
                    .position = { px, py, pz, 0.f },
                    .color = { cr, cg, cb, 1.f },
            });
        }

        for (auto idx: shapes[0].mesh.indices) {
            m_Indices.push_back(idx.vertex_index);
            float nx = attrib.normals[idx.normal_index * 3];
            float ny = attrib.normals[idx.normal_index * 3 + 1];
            float nz = attrib.normals[idx.normal_index * 3 + 2];
            m_Vertices[idx.vertex_index].normal = { nx, ny, nz, 1.f };
        }

        if (attrib.texcoords.empty()) return;

        for (auto idx : shapes[0].mesh.indices) {
            float u = 0.f, v = 0.f;
            if (idx.texcoord_index >= 0) {
                // Flip Y coord.
                u = attrib.texcoords[2 * idx.texcoord_index];
                v = 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1];
            }
            m_Vertices[idx.vertex_index].uv = { u, v };
        }
    }

    const std::vector<Vertex>& Mesh::GetVertices() const {
        return m_Vertices;
    }

    const std::vector<uint32_t>& Mesh::GetIndices() const {
        return m_Indices;
    }

    glm::mat4 Mesh::GetModelMatrix() const {
        return m_Scene->GetEntity(m_ParentId).GetTransform().GetMatrix();
    }
}
