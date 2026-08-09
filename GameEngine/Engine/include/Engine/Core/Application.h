#pragma once

#include "Engine/Core/LayerStack.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/Event.h"

#include <string>

namespace Engine
{
    /// Owns the main loop and the layer stack; the root object of any
    /// engine-based program.
    ///
    /// No Application::Get() singleton yet — nothing in this milestone
    /// needs global engine access, and adding it speculatively would trade
    /// away testability (as seen below, Tests construct plain Application
    /// instances directly) for a convenience nothing currently uses. M3's
    /// Window/Input layer may justify it; that will be a deliberate
    /// decision made then, not inherited by default from here.
    class Application
    {
    public:
        explicit Application(std::string name = "Engine Application");
        virtual ~Application() = default;

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

    private:
        bool OnWindowClose(WindowCloseEvent& event);

        std::string m_Name;
        bool m_Running = true;
        float m_LastFrameTime = 0.0f;
        LayerStack m_LayerStack;
    };

} // namespace Engine
