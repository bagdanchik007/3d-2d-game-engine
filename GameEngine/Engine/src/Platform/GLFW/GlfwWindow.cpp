#include "Engine/Platform/GLFW/GlfwWindow.h"

#include "Engine/Core/Assert.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/MouseCodes.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Platform/OpenGL/OpenGLContext.h"

#include <GLFW/glfw3.h>

namespace Engine
{
    namespace
    {
        // Tracks how many GlfwWindow instances are alive, so glfwInit() and
        // glfwTerminate() run exactly once regardless of how many windows
        // are created/destroyed over the application's lifetime. A single
        // global counter is an acceptable, well-contained use of global
        // state here: GLFW's own C API is itself process-global (one
        // glfwInit/glfwTerminate pair for the whole process), so this
        // mirrors a constraint imposed by the library, not one we invented.
        int s_GLFWWindowCount = 0;

        void GLFWErrorCallback(int error, const char* description)
        {
            ENGINE_CORE_ERROR("GLFW Error ({}): {}", error, description);
        }
    }

    GlfwWindow::GlfwWindow(const WindowProps& props)
    {
        Init(props);
    }

    GlfwWindow::~GlfwWindow()
    {
        Shutdown();
    }

    void GlfwWindow::Init(const WindowProps& props)
    {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        ENGINE_CORE_INFO("Creating window '{}' ({}x{})", props.Title, props.Width, props.Height);

        if (s_GLFWWindowCount == 0)
        {
            glfwSetErrorCallback(GLFWErrorCallback);
            const int success = glfwInit();
            ENGINE_CORE_ASSERT(success, "Could not initialize GLFW");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); // required for a core profile context on macOS; harmless elsewhere

        m_Window = glfwCreateWindow(
            static_cast<int>(props.Width), static_cast<int>(props.Height), m_Data.Title.c_str(), nullptr, nullptr);
        ++s_GLFWWindowCount;

        m_Context = std::make_unique<OpenGLContext>(m_Window);
        m_Context->Init();

        glfwSetWindowUserPointer(m_Window, &m_Data);
        SetVSync(true);

        // --- GLFW callbacks: translate C-API callbacks into Engine::Event
        // instances and forward them through the window's event callback.
        // Every callback recovers WindowData via glfwGetWindowUserPointer
        // because that pointer is the only piece of engine state GLFW's C
        // callbacks can carry.

        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
        {
            auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.Width = static_cast<uint32_t>(width);
            data.Height = static_cast<uint32_t>(height);

            WindowResizeEvent event(data.Width, data.Height);
            if (data.EventCallback)
            {
                data.EventCallback(event);
            }
        });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
        {
            auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            WindowCloseEvent event;
            if (data.EventCallback)
            {
                data.EventCallback(event);
            }
        });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
        {
            auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data.EventCallback)
            {
                return;
            }

            const auto keyCode = static_cast<KeyCode>(key);
            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event(keyCode, false);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(keyCode, true);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(keyCode);
                    data.EventCallback(event);
                    break;
                }
                default:
                    break;
            }
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int /*mods*/)
        {
            auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data.EventCallback)
            {
                return;
            }

            const auto mouseCode = static_cast<MouseCode>(button);
            if (action == GLFW_PRESS)
            {
                MouseButtonPressedEvent event(mouseCode);
                data.EventCallback(event);
            }
            else if (action == GLFW_RELEASE)
            {
                MouseButtonReleasedEvent event(mouseCode);
                data.EventCallback(event);
            }
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
        {
            auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (data.EventCallback)
            {
                MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
                data.EventCallback(event);
            }
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
        {
            auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (data.EventCallback)
            {
                MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
                data.EventCallback(event);
            }
        });
    }

    void GlfwWindow::Shutdown()
    {
        glfwDestroyWindow(m_Window);
        --s_GLFWWindowCount;

        if (s_GLFWWindowCount == 0)
        {
            glfwTerminate();
        }
    }

    void GlfwWindow::OnUpdate()
    {
        glfwPollEvents();
        m_Context->SwapBuffers();
    }

    void GlfwWindow::SetVSync(bool enabled)
    {
        glfwSwapInterval(enabled ? 1 : 0);
        m_Data.VSync = enabled;
    }

    std::unique_ptr<Window> Window::Create(const WindowProps& props)
    {
        return std::make_unique<GlfwWindow>(props);
    }

} // namespace Engine
