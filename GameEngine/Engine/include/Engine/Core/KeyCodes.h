#pragma once

#include <cstdint>

namespace Engine
{
    /// Engine-owned key codes so client code never needs to
    /// `#include <GLFW/glfw3.h>` just to write `Key::Space`.
    ///
    /// Numeric values are deliberately identical to GLFW's own key codes.
    /// This is not laziness: it means GlfwWindow's event translation layer
    /// is a direct static_cast, with zero lookup table to build, maintain,
    /// or get out of sync. If a second backend is ever added (e.g. SDL),
    /// *that* backend absorbs the cost of a translation table — Engine's
    /// public API stays exactly as it is now.
    enum class KeyCode : uint16_t
    {
        Space      = 32,
        Apostrophe = 39,
        Comma      = 44,
        Minus      = 45,
        Period     = 46,
        Slash      = 47,

        D0 = 48, D1 = 49, D2 = 50, D3 = 51, D4 = 52,
        D5 = 53, D6 = 54, D7 = 55, D8 = 56, D9 = 57,

        Semicolon = 59,
        Equal     = 61,

        A = 65, B = 66, C = 67, D = 68, E = 69, F = 70, G = 71,
        H = 72, I = 73, J = 74, K = 75, L = 76, M = 77, N = 78,
        O = 79, P = 80, Q = 81, R = 82, S = 83, T = 84, U = 85,
        V = 86, W = 87, X = 88, Y = 89, Z = 90,

        Escape    = 256,
        Enter     = 257,
        Tab       = 258,
        Backspace = 259,
        Right     = 262,
        Left      = 263,
        Down      = 264,
        Up        = 265,

        LeftShift = 340,
        LeftControl = 341,
        LeftAlt   = 342,
        RightShift = 344,
        RightControl = 345,
        RightAlt  = 346,
    };

} // namespace Engine
