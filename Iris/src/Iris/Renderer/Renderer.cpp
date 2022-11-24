#include "Renderer.hpp"
#include "Iris/Platform/Vulkan/VulkanRenderer.hpp"
#include "Iris/Platform/OpenGL/OpenGLRenderer.hpp"

namespace Iris {
    Renderer::Renderer(std::shared_ptr<Window> window)
    : m_Window(std::move(window)) {
    }

    Renderer::~Renderer() = default;

    std::shared_ptr<Renderer> Renderer::Create(RenderAPI api, const std::shared_ptr<Window>& window) {
        switch (api) {
            case RenderAPI::Vulkan:
                return std::make_shared<VulkanRenderer>(window);
            case RenderAPI::OpenGL:
                return std::make_shared<OpenGLRenderer>(window);
            default:
                return {};
        }
    }
}