#pragma once

#include "Engine/Core/Application.h"
#include "Engine/Core/Layer.h"
#include "Engine/Core/Log.h"

namespace Sandbox
{
    /// Demonstrates the Layer lifecycle. Takes Application& explicitly
    /// (constructor injection) rather than reaching for a global
    /// Application::Get() — see Application.h for why that singleton
    /// doesn't exist yet.
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
            ENGINE_INFO("ExampleLayer::OnUpdate - frame {}, dt = {:.4f} ms", m_FrameCount, timestep.GetMilliseconds());

            constexpr int kFramesBeforeClosing = 5;
            if (m_FrameCount >= kFramesBeforeClosing)
            {
                ENGINE_INFO("ExampleLayer requesting application close after {} frames", m_FrameCount);
                m_App.Close();
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
