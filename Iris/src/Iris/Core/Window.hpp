#pragma once
#include "GLFW/glfw3.h"

namespace Iris {
    class Window {
    public:
        Window(std::string_view title, glm::ivec2 size);
        ~Window();

        bool ShouldClose();
    private:
        GLFWwindow* m_Window;
        std::string m_Title;
        glm::ivec2 m_Size;
    };
}
