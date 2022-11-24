#include "Renderer.hpp"
#include "Iris/Platform/Vulkan/VulkanRenderer.hpp"

namespace Iris {
    Renderer::Renderer(const WindowOptions& opts)
            : m_Window(opts) {
    }

    Renderer::~Renderer() = default;

    std::shared_ptr<Renderer> Renderer::Create(RenderAPI api, const WindowOptions& opts) {
        switch (api) {
            case RenderAPI::Vulkan:
                return std::make_shared<VulkanRenderer>(opts);
            default:
                return {};
        }
    }
}