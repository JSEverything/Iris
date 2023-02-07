#include "Renderer.hpp"
#include "Iris/Platform/Vulkan/Renderer.hpp"
#include "Iris/Platform/OpenGL/OpenGLRenderer.hpp"
#include "Iris/Util/Input.hpp"

using namespace std::chrono_literals;

namespace Iris {
    Renderer::Renderer(const std::shared_ptr<Window>& window) : m_Window(window) {
        Log::Core::Info("Renderer {} created", window->GetTitle());
        if (m_Window) {
            m_Size.x = window->GetWidth();
            m_Size.y = window->GetHeight();
        }
    }

    std::unique_ptr<Renderer>
    Renderer::Create(RenderAPI api, const std::shared_ptr<Window>& window) {
        switch (api) {
            case RenderAPI::Vulkan:
                return std::make_unique<Vulkan::Renderer>(window);
            case RenderAPI::OpenGL:
                return std::make_unique<OpenGLRenderer>(window);
            default:
                return {};
        }
    }

    void Renderer::SetScene(const std::shared_ptr<Scene>& scene) {
        m_Scene = scene;
    }

    void Renderer::Present() {
        ++m_FrameNr;

        if (m_Window) {
            glfwSwapBuffers(m_Window->GetGLFWWindow());
        }
    }
}