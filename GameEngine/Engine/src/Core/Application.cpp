#include "Engine/Core/Application.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/Time.h"

namespace Engine
{
    Application::Application(std::string name)
        : m_Name(std::move(name))
    {
        Time::Init();
        ENGINE_CORE_INFO("Application '{}' constructed", m_Name);
    }

    void Application::Run()
    {
        ENGINE_CORE_INFO("Application '{}' entering main loop", m_Name);

        while (m_Running)
        {
            const float time = Time::GetSeconds();
            const Timestep timestep(time - m_LastFrameTime);
            m_LastFrameTime = time;

            // Update order: bottom of the stack to top (regular layers
            // first, overlays last) — an overlay drawing debug UI should
            // see the world state *after* game layers have updated it.
            for (auto& layer : m_LayerStack)
            {
                layer->OnUpdate(timestep);
            }
        }

        ENGINE_CORE_INFO("Application '{}' exiting main loop", m_Name);
    }

    void Application::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowCloseEvent>(
            [this](WindowCloseEvent& e) { return OnWindowClose(e); });

        // Event order: top of the stack to bottom (overlays first) — an
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

} // namespace Engine
