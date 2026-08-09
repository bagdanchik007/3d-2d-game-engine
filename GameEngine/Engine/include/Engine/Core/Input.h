#pragma once

#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/MouseCodes.h"

#include <utility>

namespace Engine
{
    /// Polling-based input queries, callable from anywhere in game code
    /// (physics, gameplay systems) without threading a Window& through
    /// every call site — see the rationale for Application::Get() in
    /// Application.h, which this class relies on to reach the current
    /// window's native handle.
    ///
    /// This complements, rather than replaces, the KeyPressedEvent/
    /// KeyReleasedEvent event stream: events are for "something just
    /// happened" (open a menu on a single keypress), polling is for
    /// "what's true right now" (is the player still holding W this frame).
    /// Using events for continuous movement produces frame-rate-dependent
    /// stutter; using polling for one-shot actions risks missing fast
    /// press-release cycles between polls. Both have a place.
    class Input
    {
    public:
        Input() = delete;

        [[nodiscard]] static bool IsKeyPressed(KeyCode key);
        [[nodiscard]] static bool IsMouseButtonPressed(MouseCode button);
        [[nodiscard]] static std::pair<float, float> GetMousePosition();
        [[nodiscard]] static float GetMouseX();
        [[nodiscard]] static float GetMouseY();
    };

} // namespace Engine
