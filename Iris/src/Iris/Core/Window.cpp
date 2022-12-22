#include "Window.hpp"
#include "Iris/Util/Input.hpp"

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

        double x, y;
        glfwGetCursorPos(m_Window, &x, &y);
        m_PreviousCursorPos = { x, y };

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
            auto input = Input::Get();
            switch (static_cast<KeyAction>(action)) {
                case KeyAction::PRESS:
                    input.emit<KeyPress>(key, KeyMods(mods));
                    input.emit<Key>(key, KeyMods(mods));
                    break;
                case KeyAction::RELEASE:
                    input.emit<KeyRelease>(key, KeyMods(mods));
                    break;
                case KeyAction::REPEAT:
                    input.emit<KeyRepeat>(key, KeyMods(mods));
                    input.emit<Key>(key, KeyMods(mods));
                    break;
            }
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* GLFWWindow, int button, int action, int mods) {
            auto input = Input::Get();
            switch (static_cast<KeyAction>(action)) {
                case KeyAction::PRESS:
                    input.emit<KeyPress>(button, KeyMods(mods));
                    input.emit<Key>(button, KeyMods(mods));
                    break;
                case KeyAction::RELEASE:
                    input.emit<KeyRelease>(button, KeyMods(mods));
                    break;
                case KeyAction::REPEAT:
                    input.emit<KeyRepeat>(button, KeyMods(mods));
                    input.emit<Key>(button, KeyMods(mods));
                    break;
            }
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* GLFWWindow, double x, double y) {
            auto* that = static_cast<Window*>(glfwGetWindowUserPointer(GLFWWindow));
            auto input = Input::Get();
            input.emit<MouseMove>(that->m_PreviousCursorPos - glm::vec2{ x, y });
            that->m_PreviousCursorPos = { x, y };
            input.emit<MousePosition>(glm::vec2{ x, y });
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* GLFWWindow, double x, double y) {
            auto input = Input::Get();
            input.emit<MouseScroll>(glm::vec2(x, y));
        });
    }

    Window::~Window() {
        glfwDestroyWindow(m_Window);
    }

    bool Window::ShouldClose() {
        return glfwWindowShouldClose(m_Window);
    }
}