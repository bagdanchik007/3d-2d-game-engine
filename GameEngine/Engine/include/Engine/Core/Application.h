#pragma once

#include "Engine/Core/LayerStack.h"
#include "Engine/Core/Window.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/Event.h"

#include <functional>
#include <memory>
#include <string>

namespace Engine
{
    /// Owns the main loop, the window, and the layer stack; the root object
    /// of any engine-based program.
    ///
    /// Application::Get() exists now (it deliberately did not in M2) because
    /// Input polling (see Input.h) needs the current window's native handle
    /// from potentially anywhere in game code, without threading a
    /// Window& through every call site. That is a real, present need, not
    /// a speculative one — the justification for a singleton is supposed
    /// to expire the moment nothing needs it, and something now does.
    class Application
    {
    public:
        /// Function that creates a Window given its properties. Defaults to
        /// the real GLFW backend (Window::Create). Tests pass a different
        /// factory (see Tests/TestSupport/NullWindow.h) to construct an
        /// Application without a display server.
        ///
        /// This is dependency injection via a stored function, not a
        /// virtual method Application calls on itself: a virtual call made
        /// from inside Application's own constructor would NOT dispatch to
        /// a derived override anyway (the vtable still points at Application
        /// while its own constructor is running) - a classic C++ pitfall
        /// that a first version of this class walked straight into before
        /// this was caught during test verification. A factory parameter,
        /// supplied by the caller before construction begins, has no such
        /// timing problem.
        using WindowFactory = std::function<std::unique_ptr<Window>(const WindowProps&)>;

        explicit Application(std::string name = "Engine Application", WindowFactory windowFactory = &Window::Create);
        virtual ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        /// Runs the main loop until Close() is called (directly, or
        /// indirectly via a WindowCloseEvent reaching OnEvent).
        void Run();

        /// Routes an event through OnWindowClose first, then to every layer
        /// top-to-bottom (reverse of update order) until one marks it
        /// Handled.
        void OnEvent(Event& event);

        Layer* PushLayer(std::unique_ptr<Layer> layer);
        Layer* PushOverlay(std::unique_ptr<Layer> overlay);

        void Close() noexcept { m_Running = false; }

        [[nodiscard]] bool IsRunning() const noexcept { return m_Running; }
        [[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
        [[nodiscard]] Window& GetWindow() const { return *m_Window; }

        [[nodiscard]] static Application& Get() noexcept { return *s_Instance; }

    private:
        bool OnWindowClose(WindowCloseEvent& event);
        bool OnWindowResize(WindowResizeEvent& event);

        std::string m_Name;
        bool m_Running = true;
        bool m_Minimized = false;
        float m_LastFrameTime = 0.0f;
        std::unique_ptr<Window> m_Window;
        LayerStack m_LayerStack;

        static Application* s_Instance;
    };

} // namespace Engine
