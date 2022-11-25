#pragma once
#include "Iris/Renderer/RenderAPI.hpp"
#include <glad/glad.h>
#include "GLFW/glfw3.h"

namespace Iris {
    struct WindowOptions {
        std::string_view Title;
        glm::ivec2 Size;
    };

    class Window {
    public:
        explicit Window(RenderAPI api, const WindowOptions& opts);
        ~Window();

        bool ShouldClose();

        [[nodiscard]] GLFWwindow* GetGLFWWindow() const { return m_Window; }

        [[nodiscard]] uint32_t GetWidth() const { return m_Size.x; };

        [[nodiscard]] uint32_t GetHeight() const { return m_Size.y; };

        [[nodiscard]] std::string_view GetTitle() const { return m_Title; };

        [[nodiscard]] RenderAPI GetAPI() const { return m_API; };
    private:
        GLFWwindow* m_Window;
        std::string m_Title;
        glm::ivec2 m_Size;
        RenderAPI m_API;
    };
}
