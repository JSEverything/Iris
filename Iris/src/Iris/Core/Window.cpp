#include "Window.hpp"

namespace Iris {
    Window::Window(RenderAPI api, const WindowOptions& opts)
            : m_Title(opts.Title), m_Size(opts.Size), m_API(api) {
        if (m_API == RenderAPI::OpenGL) {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        } else {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        m_Window = glfwCreateWindow(m_Size.x, m_Size.y, m_Title.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(m_Window, this);

        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* GLFWWindow, int width, int height) {
            Window& that = *(Window*)glfwGetWindowUserPointer(GLFWWindow);
            //that.emit(EventType::WindowResize, width, height);
        });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* GLFWWindow) {
            Window& that = *(Window*)glfwGetWindowUserPointer(GLFWWindow);
            //that.emit(EventType::WindowClose);
        });
    }

    Window::~Window() {
        glfwDestroyWindow(m_Window);
    }

    bool Window::ShouldClose() {
        return glfwWindowShouldClose(m_Window);
    }
}