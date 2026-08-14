#include "Engine/Scene/Scene.h"

#include "Engine/Core/Log.h"
#include "Engine/ECS/View.h"

#include <algorithm>

namespace Engine::Scene
{
    ECS::Entity Scene::CreateEntity(const std::string& name)
    {
        return CreateEntityWithID(UUID(), name);
    }

    ECS::Entity Scene::CreateEntityWithID(UUID id, const std::string& name)
    {
        const ECS::Entity entity = m_Registry.Create();
        m_Registry.AddComponent<IDComponent>(entity, id);
        m_Registry.AddComponent<TagComponent>(entity, name);
        m_Registry.AddComponent<TransformComponent>(entity);
        m_Registry.AddComponent<RelationshipComponent>(entity);
        return entity;
    }

    void Scene::DestroyEntity(ECS::Entity entity)
    {
        if (!m_Registry.IsValid(entity))
        {
            return;
        }

        // Recurse on a COPY of the children list, not the live one:
        // RemoveFromParentsChildren (called for each child as it's
        // destroyed further down this same call) mutates this entity's
        // own Children vector, and iterating a vector while something else
        // shrinks it out from under the iterator is undefined behavior -
        // exactly the kind of bug that only shows up with 3+ children,
        // never with the 1-child case a less careful test might rely on.
        const std::vector<ECS::Entity> childrenCopy = GetChildren(entity);
        for (ECS::Entity child : childrenCopy)
        {
            DestroyEntity(child);
        }

        RemoveFromParentsChildren(entity);
        m_Registry.Destroy(entity);
    }

    bool Scene::IsDescendantOf(ECS::Entity potentialDescendant, ECS::Entity ancestor) const
    {
        ECS::Entity current = potentialDescendant;
        while (!current.IsNull())
        {
            if (current == ancestor)
            {
                return true;
            }
            current = GetParent(current);
        }
        return false;
    }

    void Scene::RemoveFromParentsChildren(ECS::Entity entity)
    {
        const ECS::Entity parent = GetParent(entity);
        if (parent.IsNull() || !m_Registry.IsValid(parent))
        {
            return;
        }

        auto& parentRelationship = m_Registry.GetComponent<RelationshipComponent>(parent);
        auto& children = parentRelationship.Children;
        children.erase(std::remove(children.begin(), children.end(), entity), children.end());
    }

    void Scene::SetParent(ECS::Entity child, ECS::Entity newParent)
    {
        if (!m_Registry.IsValid(child))
        {
            ENGINE_CORE_WARN("Scene::SetParent called with an invalid child entity - ignored");
            return;
        }

        if (!newParent.IsNull())
        {
            if (child == newParent)
            {
                ENGINE_CORE_WARN("Scene::SetParent: an entity cannot be its own parent - ignored");
                return;
            }

            // Would `newParent` become a descendant of `child` through
            // this reparent? That's exactly a cycle: child's own subtree
            // would end up containing something that is now also child's
            // ancestor. Checked by walking UP from newParent looking for
            // child, which is equivalent to asking "is newParent currently
            // a descendant of child" without needing a separate downward
            // traversal.
            if (IsDescendantOf(newParent, child))
            {
                ENGINE_CORE_WARN("Scene::SetParent: rejected - would create a cycle in the transform hierarchy");
                return;
            }
        }

        RemoveFromParentsChildren(child);

        auto& childRelationship = m_Registry.GetComponent<RelationshipComponent>(child);
        childRelationship.Parent = newParent;

        if (!newParent.IsNull())
        {
            m_Registry.GetComponent<RelationshipComponent>(newParent).Children.push_back(child);
        }
    }

    ECS::Entity Scene::GetParent(ECS::Entity entity) const
    {
        if (!m_Registry.IsValid(entity) || !m_Registry.HasComponent<RelationshipComponent>(entity))
        {
            return ECS::NullEntity;
        }
        return m_Registry.GetComponent<RelationshipComponent>(entity).Parent;
    }

    const std::vector<ECS::Entity>& Scene::GetChildren(ECS::Entity entity) const
    {
        static const std::vector<ECS::Entity> kEmpty;
        if (!m_Registry.IsValid(entity) || !m_Registry.HasComponent<RelationshipComponent>(entity))
        {
            return kEmpty;
        }
        return m_Registry.GetComponent<RelationshipComponent>(entity).Children;
    }

    Math::Mat4 Scene::GetWorldTransform(ECS::Entity entity) const
    {
        if (!m_Registry.IsValid(entity) || !m_Registry.HasComponent<TransformComponent>(entity))
        {
            return Math::Mat4::Identity();
        }

        const Math::Mat4 localMatrix = m_Registry.GetComponent<TransformComponent>(entity).GetLocalMatrix();

        const ECS::Entity parent = GetParent(entity);
        if (parent.IsNull())
        {
            return localMatrix;
        }

        // Recursive, not iterative: hierarchy depth in any realistic scene
        // is small (tens of levels at most), so the call-stack cost is
        // irrelevant, and the recursive form directly mirrors the
        // recursive definition of "world transform" (parent's world
        // transform, times this entity's local transform) without needing
        // a manually-managed stack to walk ancestors bottom-up first.
        return GetWorldTransform(parent) * localMatrix;
    }

    ECS::Entity Scene::FindEntityByID(UUID id) const
    {
        // O(n) linear scan over every entity, not an indexed UUID->Entity
        // map: this is called by SceneSerializer while resolving parent
        // references during a load, not per-frame - the same "no profiling
        // evidence this needs to be faster" reasoning PhysicsWorld's broad
        // phase (M10) and Mesh's non-deduplicated vertex loading (M9)
        // already apply elsewhere in this engine.
        ECS::Entity result = ECS::NullEntity;

        // const_cast justified narrowly here: View<T> requires a non-const
        // Registry& because Each()'s callback signature always offers
        // mutable component references (see View.h) - there is no
        // const-Registry overload of View today, since nothing before
        // this method ever needed read-only iteration. FindEntityByID
        // itself only reads through the callback (never writes to the
        // IDComponent it inspects), so this is safe in practice; adding a
        // real const-correct View overload is the more principled fix if
        // a second read-only use case shows up.
        ECS::View<IDComponent> view(const_cast<ECS::Registry&>(m_Registry));
        view.Each([&](ECS::Entity entity, IDComponent& idComponent)
        {
            if (idComponent.ID == id)
            {
                result = entity;
            }
        });

        return result;
    }

} // namespace Engine::Scene
