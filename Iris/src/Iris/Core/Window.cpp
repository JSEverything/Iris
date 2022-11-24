#include "Window.hpp"

namespace Iris {
    static uint32_t windowCount = 0;

    Window::Window(const WindowOptions& opts)
            : m_Title(opts.Title), m_Size(opts.Size) {
        if (windowCount == 0) {
            glfwInit();
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_Window = glfwCreateWindow(m_Size.x, m_Size.y, m_Title.c_str(), nullptr, nullptr);
        ++windowCount;

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
        --windowCount;

        if (windowCount == 0) {
            glfwTerminate();
        }
    }

    bool Window::ShouldClose() {
        return glfwWindowShouldClose(m_Window);
    }
}