#include "Engine/Core/Application.h"
#include "Engine/Core/Log.h"
#include "ExampleLayer.h"

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
        }
    };

} // namespace Sandbox

int main()
{
    Engine::Log::Init();

    Sandbox::SandboxApp app;
    app.Run();

    return 0;
}
