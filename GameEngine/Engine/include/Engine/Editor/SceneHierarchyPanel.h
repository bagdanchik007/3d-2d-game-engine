#pragma once

#include "Engine/ECS/Entity.h"
#include "Engine/Scene/Scene.h"

namespace Engine::Editor
{
    /// Renders a Scene's transform hierarchy as an ImGui tree, tracking
    /// which entity is currently selected. Selection is exposed (not kept
    /// private) specifically so InspectorPanel can be driven by it - see
    /// EditorLayer (Sandbox) for how the two panels are wired together
    /// without either panel needing to know the other exists.
    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() = default;
        explicit SceneHierarchyPanel(Scene::Scene* context) noexcept : m_Context(context) {}

        void SetContext(Scene::Scene* context) noexcept
        {
            m_Context = context;
            m_SelectedEntity = ECS::NullEntity; // a selection from a previous scene means nothing in a new one
        }

        void OnImGuiRender();

        [[nodiscard]] ECS::Entity GetSelectedEntity() const noexcept { return m_SelectedEntity; }
        void SetSelectedEntity(ECS::Entity entity) noexcept { m_SelectedEntity = entity; }

    private:
        void DrawEntityNode(ECS::Entity entity);

        Scene::Scene* m_Context = nullptr;
        ECS::Entity m_SelectedEntity = ECS::NullEntity;
    };

} // namespace Engine::Editor
