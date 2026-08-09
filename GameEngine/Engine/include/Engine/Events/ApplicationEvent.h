#pragma once

#include "Engine/Events/Event.h"

#include <sstream>

namespace Engine
{
    class WindowCloseEvent final : public Event
    {
    public:
        WindowCloseEvent() = default;

        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class WindowResizeEvent final : public Event
    {
    public:
        WindowResizeEvent(unsigned int width, unsigned int height) noexcept
            : m_Width(width), m_Height(height)
        {
        }

        [[nodiscard]] unsigned int GetWidth() const noexcept { return m_Width; }
        [[nodiscard]] unsigned int GetHeight() const noexcept { return m_Height; }

        [[nodiscard]] std::string ToString() const override
        {
            std::ostringstream oss;
            oss << "WindowResizeEvent: " << m_Width << "x" << m_Height;
            return oss.str();
        }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        unsigned int m_Width;
        unsigned int m_Height;
    };

} // namespace Engine
