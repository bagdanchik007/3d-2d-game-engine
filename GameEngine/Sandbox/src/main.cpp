#include "Engine/Core/Application.h"
#include "Engine/Core/Log.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/UI/ImGuiLayer.h"
#include "AssetManagerLayer.h"
#include "EditorLayer.h"
#include "ExampleLayer.h"
#include "Mesh3DLayer.h"
#include "PhysicsDemoLayer.h"
#include "Renderer2DLayer.h"
#include "SceneHierarchyLayer.h"
#include "TriangleLayer.h"

namespace Sandbox
{
    class SandboxApp final : public Engine::Application
    {
    public:
        SandboxApp()
            : Application("Sandbox")
        {
            // *this refers to the already-constructed Application base
            // subobject here, so ExampleLayer's stored Application&
            // reference is valid for its entire lifetime.
            PushLayer(std::make_unique<ExampleLayer>(*this));
            PushLayer(std::make_unique<TriangleLayer>());
            PushLayer(std::make_unique<Renderer2DLayer>());
            PushLayer(std::make_unique<Mesh3DLayer>());
            PushLayer(std::make_unique<AssetManagerLayer>());
            PushLayer(std::make_unique<PhysicsDemoLayer>());
            PushLayer(std::make_unique<SceneHierarchyLayer>());
            PushLayer(std::make_unique<EditorLayer>());

            // Pushed as an overlay (see LayerStack/Layer.h, M2): must sit
            // above every other layer so it gets first refusal on events
            // (blocking mouse/keyboard input ImGui itself wants, per
            // ImGuiLayer::SetBlockEvents) before any game layer beneath it
            // sees them.
            Engine::Layer* imguiLayer = PushOverlay(std::make_unique<Engine::UI::ImGuiLayer>());
            SetImGuiLayer(imguiLayer);
        }
    };

} // namespace Sandbox

int main()
{
    Engine::Log::Init();

    Sandbox::SandboxApp app;

    // Deliberately NOT called from inside Application's constructor: doing
    // so would call real GL functions even when Application is constructed
    // with the headless NullWindow factory used by Tests/ (see
    // Tests/src/TestSupport/NullWindow.h), which never loads GL function
    // pointers at all - that would crash instead of skip. Only an app that
    // actually intends to render calls this, and only after its window
    // (and GL context) already exist.
    Engine::RenderCommand::Init();
    Engine::Renderer2D::Init();

    app.Run();

    Engine::Renderer2D::Shutdown();

    return 0;
}
