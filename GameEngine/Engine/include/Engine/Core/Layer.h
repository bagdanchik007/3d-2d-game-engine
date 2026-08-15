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

        /// Called once per frame in a SEPARATE pass, after every layer's
        /// OnUpdate has already run (see Application::Run) - layers submit
        /// ImGui widgets here, not in OnUpdate, so game-logic updates and
        /// UI submission stay cleanly separated in time even though ImGui
        /// itself is immediate-mode. Empty by default: an ordinary layer
        /// with no editor UI pays nothing for this existing.
        virtual void OnImGuiRender() {}

        /// Hooks for the ONE layer Application designates as the ImGui
        /// layer via Application::SetImGuiLayer (see Application.h) -
        /// called immediately before and after the OnImGuiRender pass,
        /// respectively. Empty for every ordinary layer; only
        /// Engine::UI::ImGuiLayer overrides these, to call
        /// ImGui::NewFrame()/backend NewFrame() here and
        /// ImGui::Render()/backend RenderDrawData() there. Kept as generic
        /// Layer virtuals rather than ImGui-specific Application API so
        /// that Application itself never needs to include anything
        /// ImGui-related - see the M12 architecture note in
        /// Application.h's SetImGuiLayer for why that matters for
        /// headless testability.
        virtual void OnImGuiFrameBegin() {}
        virtual void OnImGuiFrameEnd() {}

        [[nodiscard]] const std::string& GetName() const noexcept { return m_DebugName; }

    protected:
        std::string m_DebugName;
    };

} // namespace Engine
