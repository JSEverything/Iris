#pragma once
#include "GLFW/glfw3.h"

namespace Iris {
    struct WindowOptions {
        std::string_view Title;
        glm::ivec2 Size;
    };

    class Window {
    public:
        explicit Window(const WindowOptions& opts);
        ~Window();

        bool ShouldClose();

        GLFWwindow* GetGLFWWindow() { return m_Window; }

        uint32_t GetWidth() { return m_Size.x; };

        uint32_t GetHeight() { return m_Size.y; };

        [[nodiscard]] std::string_view GetTitle() const { return m_Title; };
    private:
        GLFWwindow* m_Window;
        std::string m_Title;
        glm::ivec2 m_Size;
    };
}
