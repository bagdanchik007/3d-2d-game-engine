#pragma once

#include "Engine/ECS/Entity.h"
#include "Engine/Scene/Scene.h"

namespace Engine::Editor
{
    /// Renders the components of a single selected entity as editable
    /// ImGui widgets. Which entity is "selected" is owned by
    /// SceneHierarchyPanel, not duplicated here - InspectorPanel is
    /// handed the entity to draw each frame rather than tracking its own
    /// notion of selection, so the two panels can never disagree about
    /// what's currently selected.
    class InspectorPanel
    {
    public:
        InspectorPanel() = default;
        explicit InspectorPanel(Scene::Scene* context) noexcept : m_Context(context) {}

        void SetContext(Scene::Scene* context) noexcept { m_Context = context; }

        void OnImGuiRender(ECS::Entity selectedEntity);

    private:
        void DrawTagComponent(ECS::Entity entity) const;
        void DrawTransformComponent(ECS::Entity entity) const;

        Scene::Scene* m_Context = nullptr;
    };

} // namespace Engine::Editor
