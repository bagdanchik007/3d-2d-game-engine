#pragma once

#include "Engine/Core/Application.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/Layer.h"
#include "Engine/Core/Log.h"
#include "Engine/Events/Event.h"

namespace Sandbox
{
    /// Demonstrates the Layer lifecycle plus real window input: press
    /// Escape (polled) or close the window (event) to exit.
    class ExampleLayer final : public Engine::Layer
    {
    public:
        explicit ExampleLayer(Engine::Application& app)
            : Layer("Example"), m_App(app)
        {
        }

        void OnAttach() override
        {
            ENGINE_INFO("ExampleLayer attached");
        }

        void OnUpdate(Engine::Timestep timestep) override
        {
            ++m_FrameCount;

            // Log sparsely - this runs once per swapchain-paced frame, but
            // we don't want to flood the console every single frame.
            constexpr int kLogEveryNFrames = 60;
            if (m_FrameCount % kLogEveryNFrames == 1)
            {
                ENGINE_INFO("ExampleLayer::OnUpdate - frame {}, dt = {:.4f} ms", m_FrameCount, timestep.GetMilliseconds());
            }

            if (Engine::Input::IsKeyPressed(Engine::KeyCode::Escape))
            {
                ENGINE_INFO("Escape pressed - requesting application close");
                m_App.Close();
            }
        }

        void OnEvent(Engine::Event& event) override
        {
            // MouseMoved fires far too often to log at info level without
            // drowning out everything else; every other event type is rare
            // enough to log unconditionally.
            if (event.GetEventType() != Engine::EventType::MouseMoved)
            {
                ENGINE_INFO("ExampleLayer::OnEvent - {}", event.ToString());
            }
        }

        void OnDetach() override
        {
            ENGINE_INFO("ExampleLayer detached");
        }

    private:
        Engine::Application& m_App;
        int m_FrameCount = 0;
    };

} // namespace Sandbox
