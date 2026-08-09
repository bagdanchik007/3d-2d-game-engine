#pragma once

#include "Engine/Core/MouseCodes.h"
#include "Engine/Events/Event.h"

#include <sstream>

namespace Engine
{
    class MouseMovedEvent final : public Event
    {
    public:
        MouseMovedEvent(float x, float y) noexcept
            : m_MouseX(x), m_MouseY(y)
        {
        }

        [[nodiscard]] float GetX() const noexcept { return m_MouseX; }
        [[nodiscard]] float GetY() const noexcept { return m_MouseY; }

        [[nodiscard]] std::string ToString() const override
        {
            std::ostringstream oss;
            oss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
            return oss.str();
        }

        EVENT_CLASS_TYPE(MouseMoved)
        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    private:
        float m_MouseX;
        float m_MouseY;
    };

    class MouseScrolledEvent final : public Event
    {
    public:
        MouseScrolledEvent(float xOffset, float yOffset) noexcept
            : m_XOffset(xOffset), m_YOffset(yOffset)
        {
        }

        [[nodiscard]] float GetXOffset() const noexcept { return m_XOffset; }
        [[nodiscard]] float GetYOffset() const noexcept { return m_YOffset; }

        [[nodiscard]] std::string ToString() const override
        {
            std::ostringstream oss;
            oss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
            return oss.str();
        }

        EVENT_CLASS_TYPE(MouseScrolled)
        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    private:
        float m_XOffset;
        float m_YOffset;
    };

    /// Shared base for the two button events, same rationale as KeyEvent.
    class MouseButtonEvent : public Event
    {
    public:
        [[nodiscard]] MouseCode GetMouseButton() const noexcept { return m_Button; }

        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)

    protected:
        explicit MouseButtonEvent(MouseCode button) noexcept
            : m_Button(button)
        {
        }

        MouseCode m_Button;
    };

    class MouseButtonPressedEvent final : public MouseButtonEvent
    {
    public:
        explicit MouseButtonPressedEvent(MouseCode button) noexcept
            : MouseButtonEvent(button)
        {
        }

        [[nodiscard]] std::string ToString() const override
        {
            std::ostringstream oss;
            oss << "MouseButtonPressedEvent: " << static_cast<uint16_t>(m_Button);
            return oss.str();
        }

        EVENT_CLASS_TYPE(MouseButtonPressed)
    };

    class MouseButtonReleasedEvent final : public MouseButtonEvent
    {
    public:
        explicit MouseButtonReleasedEvent(MouseCode button) noexcept
            : MouseButtonEvent(button)
        {
        }

        [[nodiscard]] std::string ToString() const override
        {
            std::ostringstream oss;
            oss << "MouseButtonReleasedEvent: " << static_cast<uint16_t>(m_Button);
            return oss.str();
        }

        EVENT_CLASS_TYPE(MouseButtonReleased)
    };

} // namespace Engine
