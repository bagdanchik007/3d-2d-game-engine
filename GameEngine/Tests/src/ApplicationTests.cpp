#include "Engine/Core/Application.h"
#include "Engine/Core/Log.h"
#include "Engine/Events/ApplicationEvent.h"
#include "TestSupport/NullWindow.h"

#include <catch2/catch_test_macros.hpp>

using namespace Engine;

namespace
{
    void EnsureLogInitialized()
    {
        static const bool initialized = []
        {
            Log::Init();
            return true;
        }();
        (void)initialized;
    }

    /// Application, wired to NullWindow instead of the real GLFW backend,
    /// so these tests exercise Application's own logic (event routing,
    /// layer propagation, close handling) without requiring a display
    /// server in CI.
    class HeadlessApplication final : public Application
    {
    public:
        HeadlessApplication()
            : Application("Test", &Test::NullWindow::Create)
        {
        }
    };

    /// Test double that optionally marks an event Handled, so tests can
    /// verify propagation stops at the first layer that consumes it.
    class SpyLayer final : public Layer
    {
    public:
        SpyLayer(std::string name, bool consumeEvents)
            : Layer(std::move(name)), m_ConsumeEvents(consumeEvents)
        {
        }

        void OnEvent(Event& event) override
        {
            ++m_EventsReceived;
            if (m_ConsumeEvents)
            {
                event.Handled = true;
            }
        }

        [[nodiscard]] int EventsReceived() const noexcept { return m_EventsReceived; }

    private:
        bool m_ConsumeEvents;
        int m_EventsReceived = 0;
    };
}

TEST_CASE("Application starts in the running state", "[application]")
{
    EnsureLogInitialized();
    HeadlessApplication app;

    REQUIRE(app.IsRunning());
}

TEST_CASE("A WindowCloseEvent stops the application and is marked handled", "[application]")
{
    EnsureLogInitialized();
    HeadlessApplication app;

    WindowCloseEvent closeEvent;
    app.OnEvent(closeEvent);

    REQUIRE_FALSE(app.IsRunning());
    REQUIRE(closeEvent.Handled);
}

TEST_CASE("Events propagate top-to-bottom and stop at the first layer that handles them", "[application]")
{
    EnsureLogInitialized();
    HeadlessApplication app;

    // Push order: Bottom, then Top. Top is pushed as an overlay so it sits
    // above Bottom and must receive the event first (see Application::
    // OnEvent's reverse iteration).
    auto* bottom = dynamic_cast<SpyLayer*>(
        app.PushLayer(std::make_unique<SpyLayer>("Bottom", /*consumeEvents=*/false)));
    auto* top = dynamic_cast<SpyLayer*>(
        app.PushOverlay(std::make_unique<SpyLayer>("Top", /*consumeEvents=*/true)));

    WindowResizeEvent resizeEvent(800, 600);
    app.OnEvent(resizeEvent);

    REQUIRE(top->EventsReceived() == 1);
    REQUIRE(bottom->EventsReceived() == 0); // Top consumed it; Bottom must never see it
    REQUIRE(resizeEvent.Handled);
}

TEST_CASE("An unhandled event reaches every layer in the stack", "[application]")
{
    EnsureLogInitialized();
    HeadlessApplication app;

    auto* bottom = dynamic_cast<SpyLayer*>(
        app.PushLayer(std::make_unique<SpyLayer>("Bottom", /*consumeEvents=*/false)));
    auto* top = dynamic_cast<SpyLayer*>(
        app.PushOverlay(std::make_unique<SpyLayer>("Top", /*consumeEvents=*/false)));

    WindowResizeEvent resizeEvent(800, 600);
    app.OnEvent(resizeEvent);

    REQUIRE(top->EventsReceived() == 1);
    REQUIRE(bottom->EventsReceived() == 1);
    REQUIRE_FALSE(resizeEvent.Handled);
}
