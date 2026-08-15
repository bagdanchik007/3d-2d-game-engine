#include "Engine/Editor/SceneHierarchyPanel.h"

#include "Engine/ECS/View.h"

#include <imgui.h>

#include <cstdint>
#include <vector>

namespace Engine::Editor
{
    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");

        if (m_Context != nullptr)
        {
            // Collected into a plain vector FIRST, then iterated
            // separately from the View - DrawEntityNode below can trigger
            // Scene::DestroyEntity (via the "Delete Entity" context menu),
            // which swap-removes from this same IDComponent pool. Calling
            // that while still inside View::Each's iteration over the
            // pool's dense array (see ECS/View.h) would corrupt the very
            // iteration in progress - the same class of bug
            // Scene::DestroyEntity's own "copy the children list before
            // recursing" fix (M11) exists to avoid, just one level up the
            // call stack here.
            std::vector<ECS::Entity> rootEntities;
            ECS::Registry& registry = m_Context->GetRegistry();
            ECS::View<Scene::IDComponent> view(registry);
            view.Each([this, &rootEntities](ECS::Entity entity, Scene::IDComponent&)
            {
                if (m_Context->GetParent(entity).IsNull())
                {
                    rootEntities.push_back(entity);
                }
            });

            for (ECS::Entity root : rootEntities)
            {
                DrawEntityNode(root);
            }

            // Right-click anywhere in the empty panel area (not on a
            // node) to create a new root-level entity - the ImGui
            // convention for "context menu for the panel itself" rather
            // than any specific item in it.
            if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
            {
                if (ImGui::MenuItem("Create Empty Entity"))
                {
                    m_SelectedEntity = m_Context->CreateEntity("Entity");
                }
                ImGui::EndPopup();
            }
        }

        ImGui::End();
    }

    void SceneHierarchyPanel::DrawEntityNode(ECS::Entity entity)
    {
        ECS::Registry& registry = m_Context->GetRegistry();
        const std::string& name = registry.GetComponent<Scene::TagComponent>(entity).Name;
        const auto& children = m_Context->GetChildren(entity);

        ImGuiTreeNodeFlags flags = (m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0;
        flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (children.empty())
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }

        // ImGui needs a unique ID per node independent of the displayed
        // label (two entities can share a name); entity.Index() is unique
        // among currently-alive entities, which is exactly the lifetime
        // this tree is drawn for - reusing it here avoids needing a
        // separate ImGui-specific ID scheme.
        const bool opened = ImGui::TreeNodeEx(
            reinterpret_cast<void*>(static_cast<uintptr_t>(entity.Index())), flags, "%s", name.c_str());

        if (ImGui::IsItemClicked())
        {
            m_SelectedEntity = entity;
        }

        bool entityDeleted = false;
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Delete Entity"))
            {
                entityDeleted = true;
            }
            ImGui::EndPopup();
        }

        if (opened && !(flags & ImGuiTreeNodeFlags_Leaf))
        {
            // Copied, not iterated live: `children` is a reference to the
            // parent's actual RelationshipComponent::Children vector, and
            // a nested "Delete Entity" click further down this recursion
            // triggers Scene::DestroyEntity, which mutates that exact
            // vector via RemoveFromParentsChildren while this loop would
            // otherwise still be iterating it - the same iterator-
            // invalidation hazard the collection above this function
            // avoids, one level deeper.
            const std::vector<ECS::Entity> childrenCopy = children;
            for (ECS::Entity child : childrenCopy)
            {
                DrawEntityNode(child);
            }
            ImGui::TreePop();
        }

        if (entityDeleted)
        {
            if (m_SelectedEntity == entity)
            {
                m_SelectedEntity = ECS::NullEntity;
            }
            m_Context->DestroyEntity(entity);
        }
    }

} // namespace Engine::Editor
