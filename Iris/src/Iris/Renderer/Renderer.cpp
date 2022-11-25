#include "Renderer.hpp"
#include "Iris/Platform/Vulkan/VulkanRenderer.hpp"
#include "Iris/Platform/OpenGL/OpenGLRenderer.hpp"
#include <chrono>

using namespace std::chrono_literals;

namespace Iris {
    Renderer::Renderer(RenderAPI api, const WindowOptions& opts)
            : m_Window(std::move(std::make_shared<Window>(api, opts))),
              m_Thread(std::move(std::thread(&Renderer::Run, this))) {
        Log::Core::Info("Renderer {} created", m_Window->GetTitle());
    }

    Renderer::~Renderer() {
        m_Running = false;
        m_Thread.join();
        Log::Core::Info("Renderer {} destroyed", m_Window->GetTitle());
    }

    std::shared_ptr<Renderer>
    Renderer::Create(RenderAPI api, const WindowOptions& opts) {
        switch (api) {
            case RenderAPI::Vulkan:
                return std::make_shared<VulkanRenderer>(opts);
            case RenderAPI::OpenGL:
                return std::make_shared<OpenGLRenderer>(opts);
            default:
                return {};
        }
    }

    void Renderer::Run() {
        // VFT is created AFTER the constructor for some reason
        // The constructor needs to end execution before any function calls on 'this' work.
        std::this_thread::sleep_for(10ms);

        glfwMakeContextCurrent(m_Window->GetGLFWWindow());
        Init();

        while (m_Running) {
            Draw();
            glfwSwapBuffers(m_Window->GetGLFWWindow());
            if (glfwWindowShouldClose(m_Window->GetGLFWWindow())) break;
            ++m_FrameNr;
        }
        Cleanup();
        m_Running = false;
    }
}