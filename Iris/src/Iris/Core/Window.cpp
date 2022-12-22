#include "Window.hpp"

namespace Iris {
    enum class KeyAction : int {
        PRESS = GLFW_PRESS,
        RELEASE = GLFW_RELEASE,
        REPEAT = GLFW_REPEAT,
    };

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
            auto* that = static_cast<Window*>(glfwGetWindowUserPointer(GLFWWindow));
            that->emit<WindowResize>(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* GLFWWindow) {
            auto* that = static_cast<Window*>(glfwGetWindowUserPointer(GLFWWindow));
            that->emit<WindowClose>();
        });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* GLFWWindow, int key, int scancode, int action, int mods) {
            auto* that = static_cast<Window*>(glfwGetWindowUserPointer(GLFWWindow));
            switch (static_cast<KeyAction>(action)) {
                case KeyAction::PRESS:
                    that->emit<KeyPress>(key, KeyMods(mods));
                    that->emit<Key>(key, KeyMods(mods));
                    break;
                case KeyAction::RELEASE:
                    that->emit<KeyRelease>(key, KeyMods(mods));
                    break;
                case KeyAction::REPEAT:
                    that->emit<KeyRepeat>(key, KeyMods(mods));
                    that->emit<Key>(key, KeyMods(mods));
                    break;
            }
        });
    }

    Window::~Window() {
        glfwDestroyWindow(m_Window);
    }

    bool Window::ShouldClose() {
        return glfwWindowShouldClose(m_Window);
    }
}