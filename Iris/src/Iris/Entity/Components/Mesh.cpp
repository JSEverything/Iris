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
        m_Vertices.resize(attrib.vertices.size() / 3);
        for (auto idx: shapes[0].mesh.indices) {
            m_Indices.push_back(idx.vertex_index);
            float nx = attrib.normals[idx.normal_index * 3];
            float ny = attrib.normals[idx.normal_index * 3 + 1];
            float nz = attrib.normals[idx.normal_index * 3 + 2];

            float px = attrib.vertices[idx.vertex_index * 3];
            float py = attrib.vertices[idx.vertex_index * 3 + 1];
            float pz = attrib.vertices[idx.vertex_index * 3 + 2];

            float cr = attrib.colors[idx.vertex_index * 3];
            float cg = attrib.colors[idx.vertex_index * 3 + 1];
            float cb = attrib.colors[idx.vertex_index * 3 + 2];

            m_Vertices[idx.vertex_index].position = { px, py, pz, 1.f };
            m_Vertices[idx.vertex_index].color = { cr, cg, cb, 1.f };
            m_Vertices[idx.vertex_index].normal = { nx, ny, nz, 1.f };

            float u = 0.f, v = 0.f;
            if (!attrib.texcoords.empty() && idx.texcoord_index != -1) {
                u = attrib.texcoords[idx.texcoord_index * 2];
                v = 1.0f - attrib.texcoords[idx.texcoord_index * 2 + 1]; // Flip Y coord.
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
