#pragma once

#include <cstdint>

namespace Engine
{
    /// See KeyCodes.h for why these values match GLFW's numbering exactly.
    enum class MouseCode : uint16_t
    {
        Button0 = 0,
        Button1 = 1,
        Button2 = 2,

        ButtonLeft   = Button0,
        ButtonRight  = Button1,
        ButtonMiddle = Button2,
    };

} // namespace Engine
