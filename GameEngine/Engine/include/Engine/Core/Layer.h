#pragma once

#include "Engine/Core/Timestep.h"
#include "Engine/Events/Event.h"

#include <string>
#include <utility>

namespace Engine
{
    /// Extension point for decoupled engine subsystems (game logic, editor
    /// UI, debug overlays) that all need to update and receive events
    /// without knowing about each other.
    ///
    /// This is one of the few places inheritance is used deliberately
    /// rather than composition: Layer *is* an interface in the true sense —
    /// LayerStack and Application only ever interact with layers through
    /// this base, never caring about the concrete type. That's exactly the
    /// case where a virtual interface earns its cost.
    class Layer
    {
    public:
        explicit Layer(std::string debugName = "Layer") noexcept
            : m_DebugName(std::move(debugName))
        {
        }

        virtual ~Layer() = default;

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate(Timestep timestep) { (void)timestep; }
        virtual void OnEvent(Event& event) { (void)event; }

        [[nodiscard]] const std::string& GetName() const noexcept { return m_DebugName; }

    protected:
        std::string m_DebugName;
    };

} // namespace Engine
