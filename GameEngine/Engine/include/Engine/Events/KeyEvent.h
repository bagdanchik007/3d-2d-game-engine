#pragma once

#include "Engine/Core/KeyCodes.h"
#include "Engine/Events/Event.h"

#include <sstream>

namespace Engine
{
    /// Common base for key events: every concrete key event carries a
    /// KeyCode, so the shared accessor belongs here rather than being
    /// duplicated in each subclass. This is the "real architectural value"
    /// case for a shared base — not used for polymorphic storage, purely to
    /// avoid repeating GetKeyCode() three times.
    class KeyEvent : public Event
    {
    public:
        [[nodiscard]] KeyCode GetKeyCode() const noexcept { return m_KeyCode; }

        EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard)

    protected:
        explicit KeyEvent(KeyCode keyCode) noexcept
            : m_KeyCode(keyCode)
        {
        }

        KeyCode m_KeyCode;
    };

    class KeyPressedEvent final : public KeyEvent
    {
    public:
        KeyPressedEvent(KeyCode keyCode, bool isRepeat) noexcept
            : KeyEvent(keyCode), m_IsRepeat(isRepeat)
        {
        }

        [[nodiscard]] bool IsRepeat() const noexcept { return m_IsRepeat; }

        [[nodiscard]] std::string ToString() const override
        {
            std::ostringstream oss;
            oss << "KeyPressedEvent: " << static_cast<uint16_t>(m_KeyCode) << " (repeat=" << m_IsRepeat << ")";
            return oss.str();
        }

        EVENT_CLASS_TYPE(KeyPressed)

    private:
        bool m_IsRepeat;
    };

    class KeyReleasedEvent final : public KeyEvent
    {
    public:
        explicit KeyReleasedEvent(KeyCode keyCode) noexcept
            : KeyEvent(keyCode)
        {
        }

        [[nodiscard]] std::string ToString() const override
        {
            std::ostringstream oss;
            oss << "KeyReleasedEvent: " << static_cast<uint16_t>(m_KeyCode);
            return oss.str();
        }

        EVENT_CLASS_TYPE(KeyReleased)
    };

} // namespace Engine
