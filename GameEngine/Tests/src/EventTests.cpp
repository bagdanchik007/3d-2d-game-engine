#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/Event.h"

#include <catch2/catch_test_macros.hpp>

using namespace Engine;

TEST_CASE("WindowCloseEvent reports its own type and category", "[events]")
{
    WindowCloseEvent event;

    REQUIRE(event.GetEventType() == EventType::WindowClose);
    REQUIRE(std::string(event.GetName()) == "WindowClose");
    REQUIRE(event.IsInCategory(EventCategoryApplication));
    REQUIRE_FALSE(event.Handled);
}

TEST_CASE("WindowResizeEvent carries its dimensions and formats them", "[events]")
{
    WindowResizeEvent event(1920, 1080);

    REQUIRE(event.GetWidth() == 1920);
    REQUIRE(event.GetHeight() == 1080);
    REQUIRE(event.ToString() == "WindowResizeEvent: 1920x1080");
}

TEST_CASE("EventDispatcher invokes the handler only for a matching type", "[events]")
{
    WindowCloseEvent event;
    EventDispatcher dispatcher(event);

    bool resizeHandlerCalled = false;
    const bool resizeMatched = dispatcher.Dispatch<WindowResizeEvent>(
        [&resizeHandlerCalled](WindowResizeEvent&)
        {
            resizeHandlerCalled = true;
            return true;
        });

    REQUIRE_FALSE(resizeMatched);
    REQUIRE_FALSE(resizeHandlerCalled);
    REQUIRE_FALSE(event.Handled); // wrong-type dispatch must not touch Handled

    bool closeHandlerCalled = false;
    const bool closeMatched = dispatcher.Dispatch<WindowCloseEvent>(
        [&closeHandlerCalled](WindowCloseEvent&)
        {
            closeHandlerCalled = true;
            return true;
        });

    REQUIRE(closeMatched);
    REQUIRE(closeHandlerCalled);
    REQUIRE(event.Handled);
}

TEST_CASE("EventDispatcher does not mark Handled when the handler returns false", "[events]")
{
    WindowCloseEvent event;
    EventDispatcher dispatcher(event);

    dispatcher.Dispatch<WindowCloseEvent>([](WindowCloseEvent&) { return false; });

    // The handler ran (type matched) but chose not to consume the event -
    // a later handler in the chain should still get a chance at it.
    REQUIRE_FALSE(event.Handled);
}
