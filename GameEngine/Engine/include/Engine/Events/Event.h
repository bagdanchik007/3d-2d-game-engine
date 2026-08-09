#pragma once

#include "Engine/Core/Base.h"

#include <string>

namespace Engine
{
    enum class EventType
    {
        None = 0,
        WindowClose,
        WindowResize,
        KeyPressed,
        KeyReleased,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseScrolled,
    };

    enum EventCategory
    {
        EventCategoryNone = 0,
        EventCategoryApplication = ENGINE_BIT(0),
        EventCategoryInput       = ENGINE_BIT(1),
        EventCategoryKeyboard    = ENGINE_BIT(2),
        EventCategoryMouse       = ENGINE_BIT(3),
        EventCategoryMouseButton = ENGINE_BIT(4),
    };

    /// Base class for all engine events.
    ///
    /// Virtual dispatch (not std::variant) is a deliberate choice: input
    /// events run at maybe hundreds per second, nowhere near a hot path
    /// like ECS iteration, so vtable overhead is irrelevant here. In
    /// exchange, new event types never require touching a central variant
    /// definition — each one is fully self-contained. See EventDispatcher
    /// below for how a specific event type is recovered safely.
    class Event
    {
    public:
        virtual ~Event() = default;

        [[nodiscard]] virtual EventType GetEventType() const = 0;
        [[nodiscard]] virtual const char* GetName() const = 0;
        [[nodiscard]] virtual int GetCategoryFlags() const = 0;
        [[nodiscard]] virtual std::string ToString() const { return GetName(); }

        [[nodiscard]] bool IsInCategory(EventCategory category) const
        {
            return (GetCategoryFlags() & category) != 0;
        }

        // Set by a handler that has fully consumed this event, so that
        // later layers in the stack (see LayerStack) know to skip it.
        // Public and mutable by design: EventDispatcher and Application's
        // layer-propagation loop both need to set this from outside the
        // class, and adding getter/setter ceremony around one bool would
        // not add any actual encapsulation value.
        bool Handled = false;
    };

    /// Recovers a concrete Event subtype from a base Event& and invokes a
    /// handler only if the runtime type matches — the alternative to
    /// dynamic_cast-and-check-for-null that most tutorials reach for.
    class EventDispatcher
    {
    public:
        explicit EventDispatcher(Event& event) noexcept
            : m_Event(event)
        {
        }

        template <typename T, typename F>
        bool Dispatch(const F& handler)
        {
            if (m_Event.GetEventType() == T::GetStaticType())
            {
                m_Event.Handled |= handler(static_cast<T&>(m_Event));
                return true;
            }
            return false;
        }

    private:
        Event& m_Event;
    };

} // namespace Engine

// Boilerplate every concrete event repeats verbatim: a compile-time type
// tag (GetStaticType) usable in EventDispatcher::Dispatch<T>, alongside the
// virtual GetEventType() that lets code query the type through a base
// Event&. A macro is the right tool here — this is not "logic", it's
// mechanical repetition that would otherwise invite copy-paste mistakes.
#define EVENT_CLASS_TYPE(type)                                                        \
    static Engine::EventType GetStaticType() { return Engine::EventType::type; }        \
    Engine::EventType GetEventType() const override { return GetStaticType(); }           \
    const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
    int GetCategoryFlags() const override { return category; }
