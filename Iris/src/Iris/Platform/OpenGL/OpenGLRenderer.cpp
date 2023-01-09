#include "OpenGLRenderer.hpp"

namespace Iris {
    OpenGLRenderer::OpenGLRenderer(const WindowOptions& opts, const std::shared_ptr<Scene>& scene)
            : Renderer(RenderAPI::OpenGL, opts, scene) {
        scene->on<ObjectAdd>([this](uint32_t entityId) {
            if (!m_Scene->GetEntity(entityId).GetComponents<Mesh>().empty()) {
                std::scoped_lock l(m_QueueMutex);
                m_EntityQueue.emplace_back(entityId);
            };
            if (!m_Scene->GetEntity(entityId).GetComponents<Camera>().empty()) {
                m_CameraEntityId = entityId;
            };
            if (!m_Scene->GetEntity(entityId).GetComponents<Material>().empty()) {
                std::scoped_lock l(m_QueueMutex);
                m_EntityQueue.emplace_back(entityId);
            };
        });
    }

    void OpenGLRenderer::Init() {
        glfwSwapInterval(1);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        Log::Core::Info("OpenGL {}.{}", GLVersion.major, GLVersion.minor);

        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, static_cast<GLsizei>(m_Window->GetWidth()), static_cast<GLsizei>(m_Window->GetHeight()));
        LoadShaders();
    }

    void OpenGLRenderer::LoadShaders() {
        m_ShaderProgram = std::make_shared<GLShaderProgram>();

        m_ShaderProgram->AddVertexShader("Shaders/phong.vert");
        m_ShaderProgram->AddFragmentShader("Shaders/phong.frag");
        m_ShaderProgram->Link();
        m_ShaderProgram->Use();

        m_ShaderProgram->SetUniform("lightPos", glm::vec3(0.f, 6.f, 0.f));

        m_ShaderProgram->SetUniform("model", glm::rotate(glm::mat4{ 1.0f }, glm::degrees(180.f), glm::vec3(0, 1, 0)));
        m_ShaderProgram->SetUniform("view", glm::translate(glm::mat4(1.f), { 0, 0, 0 }));
        m_ShaderProgram
                ->SetUniform("projection", glm::perspective(glm::radians(90.0f), 1600.f / 900.f, 0.0001f, 10000.0f));
        m_ShaderProgram->SetUniform("matAmbient", glm::vec3(0.0215f, 0.1745f, 0.0215f));
        m_ShaderProgram->SetUniform("matDiffuse", glm::vec3(0.07568f, 0.61424f, 0.07568f));
        m_ShaderProgram->SetUniform("matSpecular", glm::vec3(0.633, 0.727811f, 0.633f));
        m_ShaderProgram->SetUniform("matShininess", glm::vec3(0.6f));
    }

    void OpenGLRenderer::Draw() {
        if (!m_EntityQueue.empty()) {
            std::scoped_lock l(m_QueueMutex);
            for (uint32_t i = 0; i < m_EntityQueue.size(); ++i) {
                for (auto& mesh: m_Scene->GetEntity(m_EntityQueue[i]).GetComponents<Mesh>()) {
                    auto temp = GLMesh(GL_TRIANGLES, m_EntityQueue[i]);
                    temp.SetVertices(mesh.GetVertices());
                    temp.SetIndices(mesh.GetIndices());
                    m_Meshes.emplace_back(temp);
                }

                for (auto& material: m_Scene->GetEntity(m_EntityQueue[i]).GetComponents<Material>()) {
                    m_Textures.emplace_back();
                    m_Textures[m_Textures.size() - 1].loadFromFile(material.getTexture());
                }
            }
            m_EntityQueue.clear();
        }

        const float pi3 = std::numbers::pi / 3;
        glm::vec3 color = abs(glm::sin(
                glm::vec3(static_cast<float>(m_FrameNr) / 10.f) / glm::vec3(120.f) + glm::vec3(0.f, pi3, 2 * pi3)));

        glClearColor(color.r, color.g, color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (m_CameraEntityId != -1) {
            m_ShaderProgram->SetUniform("view", m_Scene->GetEntity(m_CameraEntityId).GetComponent<Camera>()
                    .GetViewMatrix());
            m_ShaderProgram
                    ->SetUniform("projection", m_Scene->GetEntity(m_CameraEntityId).GetComponent<Camera>()
                            .GetProjectionMatrix());
            m_ShaderProgram
                    ->SetUniform("cameraPos", m_Scene->GetEntity(m_CameraEntityId).GetTransform().GetTranslation());
        }

        /*for (uint32_t i = 0; i < vertices.size(); ++i) {
            auto& vert = vertices[i];
            vert.color = glm::vec4(abs(glm::sin(
                    glm::vec3(static_cast<float>(m_FrameNr + (i * 100)) / glm::vec3(120.f) +
                              glm::vec3(0.f, pi3, 2 * pi3)))), 1.f);
        };
        m_Triangle->SetVertices(vertices);*/

        for (GLMesh& obj: m_Meshes) {
            auto& object = m_Scene->GetEntity(obj.GetParentId());
            m_ShaderProgram
                    ->SetUniform("model", object.GetComponent<Mesh>().GetModelMatrix());
            m_ShaderProgram
                    ->SetUniform("modelPos", object.GetTransform().GetTranslation());
            m_ShaderProgram
                    ->SetUniform("diffuseTexture", 0u);

            if (!object.GetComponents<Material>().empty()) {
                m_Textures[0].bind(0);
            }

            m_ShaderProgram->Use();
            obj.Render();
            GLTexture::unbind();
        }
    }

    void OpenGLRenderer::Cleanup() {

    }
}
