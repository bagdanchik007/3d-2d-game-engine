#include "Engine/Core/Application.h"

#include "Engine/Core/Assert.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/Time.h"

namespace Engine
{
    Application* Application::s_Instance = nullptr;

    Application::Application(std::string name, WindowFactory windowFactory)
        : m_Name(std::move(name))
    {
        ENGINE_CORE_ASSERT(s_Instance == nullptr, "Application already exists - only one Application may exist per process");
        s_Instance = this;

        Time::Init();

        m_Window = windowFactory(WindowProps{m_Name, 1280u, 720u});
        m_Window->SetEventCallback([this](Event& event) { OnEvent(event); });

        ENGINE_CORE_INFO("Application '{}' constructed", m_Name);
    }

    Application::~Application()
    {
        s_Instance = nullptr;
    }

    void Application::Run()
    {
        ENGINE_CORE_INFO("Application '{}' entering main loop", m_Name);

        while (m_Running)
        {
            const float time = Time::GetSeconds();
            const Timestep timestep(time - m_LastFrameTime);
            m_LastFrameTime = time;

            // Skip game/layer updates while minimized: nothing is visible,
            // so spending CPU time simulating it is pure waste. The window
            // must still pump its message loop below, or the OS considers
            // it unresponsive.
            if (!m_Minimized)
            {
                for (auto& layer : m_LayerStack)
                {
                    layer->OnUpdate(timestep);
                }
            }

            m_Window->OnUpdate();
        }

        ENGINE_CORE_INFO("Application '{}' exiting main loop", m_Name);
    }

    void Application::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowCloseEvent>(
            [this](WindowCloseEvent& e) { return OnWindowClose(e); });
        dispatcher.Dispatch<WindowResizeEvent>(
            [this](WindowResizeEvent& e) { return OnWindowResize(e); });

        // Event order: top of the stack to bottom (overlays first) - an
        // overlay (e.g. a future editor UI capturing mouse focus) must get
        // first refusal on an event before it reaches game layers
        // underneath it.
        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            if (event.Handled)
            {
                break;
            }
            (*it)->OnEvent(event);
        }
    }

    Layer* Application::PushLayer(std::unique_ptr<Layer> layer)
    {
        return m_LayerStack.PushLayer(std::move(layer));
    }

    Layer* Application::PushOverlay(std::unique_ptr<Layer> overlay)
    {
        return m_LayerStack.PushOverlay(std::move(overlay));
    }

    bool Application::OnWindowClose(WindowCloseEvent& /*event*/)
    {
        Close();
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& event)
    {
        // Width or height of zero happens on minimize on some platforms;
        // treated as a distinct "minimized" state rather than a 0x0
        // viewport, which a later renderer milestone would otherwise choke
        // on when sizing framebuffers.
        m_Minimized = (event.GetWidth() == 0 || event.GetHeight() == 0);
        return false; // Not "handled": layers (e.g. a future viewport) still need to see resizes.
    }

} // namespace Engine
