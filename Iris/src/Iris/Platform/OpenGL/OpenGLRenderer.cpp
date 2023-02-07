#include "OpenGLRenderer.hpp"

namespace Iris {
    OpenGLRenderer::OpenGLRenderer(const std::shared_ptr<Window>& window) : Iris::Renderer(window) {
        Init();
    }

    void OpenGLRenderer::Init() {
        glfwSwapInterval(1);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        Log::Core::Info("OpenGL {}.{}", GLVersion.major, GLVersion.minor);

        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, static_cast<GLsizei>(m_Size.x), static_cast<GLsizei>(m_Size.y));
        LoadShaders();

        //m_Skybox = std::make_shared<GLSkybox>();
        //m_Skybox->loadFromFolder("../Assets/Skybox/sky");

        m_FramebufferTexture = std::make_shared<GLTexture>();
        m_FramebufferTexture->create({ m_Size.x, m_Size.y });
        m_DepthTexture = std::make_shared<GLTexture>();
        m_DepthTexture->createDepth({ m_Size.x, m_Size.y });

        m_FrameBuffer = std::make_shared<GLFramebuffer>();
        m_FrameBuffer->initialize(static_cast<int>(m_Size.x), static_cast<int>(m_Size.y));
        m_FrameBuffer->setTextureAsColorAttachment(0u, &*m_FramebufferTexture);
        m_FrameBuffer->setTextureAsDepthAttachment(&*m_DepthTexture);

        m_FramebufferMesh = std::make_shared<GLMesh>(GL_TRIANGLES, 0);
        m_FramebufferMesh->SetVertices(
                { Vertex{ .position={ -1.f, 1.f, 0.f, 1.f }, .uv = { 0.f, 1.f } },
                  Vertex{ .position={ 1.f, 1.f, 0.f, 1.f }, .uv = { 1.f, 1.f } },
                  Vertex{ .position={ -1.f, -1.f, 0.f, 1.f }, .uv = { 0.f, 0.f } },
                  Vertex{ .position={ 1.f, -1.f, 0.f, 1.f }, .uv = { 1.f, 0.f } }, });
        m_FramebufferMesh->SetIndices({ 0, 2, 1, 2, 3, 1 });
    }

    void OpenGLRenderer::LoadShaders() {
        m_ShaderProgram = std::make_shared<GLShaderProgram>();

        m_ShaderProgram->AddVertexShader("Shaders/phong.vert");
        m_ShaderProgram->AddFragmentShader("Shaders/phong.frag");
        m_ShaderProgram->Link();
        m_ShaderProgram->Use();

        m_ShaderProgram2 = std::make_shared<GLShaderProgram>();

        m_ShaderProgram2->AddVertexShader("Shaders/phongtex.vert");
        m_ShaderProgram2->AddFragmentShader("Shaders/phongtex.frag");
        m_ShaderProgram2->Link();
        m_ShaderProgram2->Use();

        m_FramebufferProgram = std::make_shared<GLShaderProgram>();
        m_FramebufferProgram->AddVertexShader("Shaders/fb.vert");
        m_FramebufferProgram->AddFragmentShader("Shaders/fb.frag");
        m_FramebufferProgram->Link();
        m_FramebufferProgram->Use();
    }

    void OpenGLRenderer::Draw(const Camera& camera) {
        const float pi3 = std::numbers::pi / 3;
        glm::vec3 color = abs(glm::sin(
                glm::vec3(static_cast<float>(m_FrameNr) / 10.f) / glm::vec3(120.f) + glm::vec3(0.f, pi3, 2 * pi3)));

        glClearColor(color.r, color.g, color.b, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (GLMesh& obj: m_Meshes) {
            auto& object = m_Scene->GetEntity(obj.GetParentId());

            if (!object.GetComponents<Material>().empty()) {
                m_ShaderProgram2->Use();

                m_ShaderProgram2->SetUniform("view", camera.GetViewMatrix());
                m_ShaderProgram2
                        ->SetUniform("projection", camera.GetProjectionMatrix());
                m_ShaderProgram2
                        ->SetUniform("cameraPos", camera.GetParent().GetTransform().GetTranslation());

                m_ShaderProgram2->SetUniform("lightPos", glm::vec3(0.f, 6.f, 0.f));

                m_ShaderProgram2->SetUniform("model", object.GetComponent<Mesh>().GetModelMatrix());
                m_ShaderProgram2->SetUniform("modelPos", object.GetTransform().GetTranslation());
                m_ShaderProgram2->SetUniform("diffuseTexture", 0u);

                m_Textures[0].bind(0u);
            } else {
                m_ShaderProgram->Use();

                m_ShaderProgram->SetUniform("view", camera.GetViewMatrix());
                m_ShaderProgram
                        ->SetUniform("projection", camera.GetProjectionMatrix());
                m_ShaderProgram
                        ->SetUniform("cameraPos", camera.GetParent().GetTransform().GetTranslation());

                m_ShaderProgram->SetUniform("lightPos", glm::vec3(0.f, 6.f, 0.f));

                m_ShaderProgram->SetUniform("matAmbient", glm::vec3(0.0215f, 0.1745f, 0.0215f));
                m_ShaderProgram->SetUniform("matDiffuse", glm::vec3(0.07568f, 0.61424f, 0.07568f));
                m_ShaderProgram->SetUniform("matSpecular", glm::vec3(0.633, 0.727811f, 0.633f));
                m_ShaderProgram->SetUniform("matShininess", glm::vec3(1.f));

                m_ShaderProgram->SetUniform("model", object.GetComponent<Mesh>().GetModelMatrix());
                m_ShaderProgram->SetUniform("modelPos", object.GetTransform().GetTranslation());
            }

            obj.Render();
            GLTexture::unbind();
        }
        //m_Skybox->render();
    }

    void OpenGLRenderer::TransferObjects() {
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

    void OpenGLRenderer::Render(const Camera& camera) {
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&GLFramebuffer::DefaultFramebuffer);
        if (!m_EntityQueue.empty()) {
            TransferObjects();
        }

        glViewport(0, 0, m_FrameBuffer->width(), m_FrameBuffer->height());
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        glViewport(0, 0, m_FrameBuffer->width() / 2, m_FrameBuffer->height());
        //m_FrameBuffer->bind();
        GLFramebuffer::bindDefault();
        Draw(camera);

        GLFramebuffer::bindDefault();
        glViewport(0, 0, m_FrameBuffer->width() / 2, m_FrameBuffer->height());
        m_FramebufferProgram->Use();
        m_FramebufferTexture->bind(0u);
        m_FramebufferProgram->SetUniform("fbTexture", 0u);
        m_FramebufferMesh->Render();
        GLTexture::unbind(0u);

        GLFramebuffer::bindDefault();
        glViewport(m_FrameBuffer->width() / 2, 0, m_FrameBuffer->width() / 2, m_FrameBuffer->height());
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_FramebufferProgram->Use();
        m_DepthTexture->bind(0u);
        m_FramebufferProgram->SetUniform("fbTexture", 0u);
        m_FramebufferMesh->Render();
        GLTexture::unbind(0u);
        //m_FrameBuffer->unbind();

        Present();
    }

    void OpenGLRenderer::SetScene(const std::shared_ptr<Scene>& scene) {
        Renderer::SetScene(scene);
        m_Scene->on<ObjectAdd>([this](uint32_t entityId) {
            if (!m_Scene->GetEntity(entityId).GetComponents<Mesh>().empty()
                || !m_Scene->GetEntity(entityId).GetComponents<Material>().empty()) {
                std::scoped_lock l(m_QueueMutex);
                m_EntityQueue.emplace_back(entityId);
            }
        });
    }
}
