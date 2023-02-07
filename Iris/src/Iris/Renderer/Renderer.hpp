#pragma once
#include "Iris/Renderer/RenderAPI.hpp"
#include "Iris/Core/Window.hpp"
#include "Iris/Scene/Scene.hpp"
#include "Iris/Entity/Components/Camera.hpp"

namespace Iris {
    class Renderer {
    public:
        static std::unique_ptr<Renderer>
        Create(RenderAPI api, const std::shared_ptr<Window>& window);

        virtual void SetScene(const std::shared_ptr<Scene>& scene);
        virtual void Render(const Camera& camera) = 0;

        [[nodiscard]] std::shared_ptr<Window>& GetWindow() { return m_Window; }

        virtual ~Renderer() = default;
    protected:
        explicit Renderer(const std::shared_ptr<Window>& window);
        virtual void Present();
    protected:
        glm::uvec2 m_Size{ 1600, 900 };
        uint64_t m_FrameNr = 0;
        std::shared_ptr<Window> m_Window{ nullptr };
        std::shared_ptr<Scene> m_Scene;
    };
}

