#pragma once

#include "Engine/Core/Timestep.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/Event.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Renderer/PerspectiveCamera.h"

namespace Engine
{
    /// WASD + mouse-look flycam controller. Kept entirely separate from
    /// PerspectiveCamera itself (see PerspectiveCamera.h) - this class
    /// owns input policy (which keys move forward, mouse sensitivity),
    /// the camera owns geometry. A future editor camera or a scripted
    /// cutscene camera would reuse PerspectiveCamera without reusing this
    /// controller at all.
    class PerspectiveCameraController
    {
    public:
        explicit PerspectiveCameraController(float aspectRatio) noexcept;

        void OnUpdate(Timestep timestep);
        void OnEvent(Event& event);

        [[nodiscard]] PerspectiveCamera& GetCamera() noexcept { return m_Camera; }
        [[nodiscard]] const PerspectiveCamera& GetCamera() const noexcept { return m_Camera; }

        void SetMovementSpeed(float unitsPerSecond) noexcept { m_MovementSpeed = unitsPerSecond; }

        /// Mouse-look only takes effect while this is true - a controller
        /// that always captured the mouse would make it impossible to
        /// click anything else in the window (a future editor UI, for
        /// instance), so look is opt-in per frame rather than always-on.
        void SetLookEnabled(bool enabled) noexcept { m_LookEnabled = enabled; m_HasLastMousePosition = false; }

    private:
        bool OnWindowResize(WindowResizeEvent& event);
        bool OnMouseMoved(MouseMovedEvent& event);

        PerspectiveCamera m_Camera;
        float m_MovementSpeed = 3.0f;
        float m_MouseSensitivity = 0.002f;

        bool m_LookEnabled = false;
        bool m_HasLastMousePosition = false;
        float m_LastMouseX = 0.0f;
        float m_LastMouseY = 0.0f;
    };

} // namespace Engine
