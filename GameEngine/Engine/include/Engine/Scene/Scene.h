#pragma once

#include "Engine/Core/UUID.h"
#include "Engine/ECS/Registry.h"
#include "Engine/Math/Math.h"
#include "Engine/Scene/Components.h"

#include <string>

namespace Engine::Scene
{
    /// Owns an ECS::Registry and layers scene-level concepts on top of the
    /// generic ECS: default components on creation, and a transform
    /// hierarchy (RelationshipComponent) with cycle-safe reparenting.
    ///
    /// This split mirrors ECS::Registry vs. ECS::View from M5: Registry
    /// stays a generic, domain-ignorant component store; Scene is where
    /// domain-specific policy (every entity gets an ID/Tag/Transform, a
    /// child can't become its own ancestor) lives. Registry gained no new
    /// knowledge of "scenes" or "hierarchies" to make this possible.
    class Scene
    {
    public:
        Scene() = default;

        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;

        /// Creates an entity with IDComponent (fresh random UUID),
        /// TagComponent, TransformComponent, and RelationshipComponent
        /// already attached - every entity a Scene creates has this full
        /// set unconditionally, so code elsewhere (SceneSerializer, a
        /// future editor hierarchy panel) never has to handle "an entity
        /// that exists but has no name" or "...no transform" as a special
        /// case.
        ECS::Entity CreateEntity(const std::string& name = "Entity");

        /// Destroys `entity` and, recursively, all of its descendants -
        /// matching the default behavior of Unity/Unreal/most editors:
        /// destroying a parent without an explicit choice about its
        /// children would otherwise leave orphaned entities with a
        /// RelationshipComponent::Parent pointing at a now-invalid Entity,
        /// silently corrupting the hierarchy rather than failing loudly.
        void DestroyEntity(ECS::Entity entity);

        /// Reparents `child` under `newParent` (or un-parents it if
        /// newParent is ECS::NullEntity). Rejects the reparent (logs a
        /// warning, leaves the hierarchy unchanged) if newParent is
        /// `child` itself or any existing descendant of `child` - either
        /// case would create a cycle, which every recursive hierarchy walk
        /// in this class (DestroyEntity, GetWorldTransform) assumes cannot
        /// exist and would otherwise infinite-loop on.
        void SetParent(ECS::Entity child, ECS::Entity newParent);

        [[nodiscard]] ECS::Entity GetParent(ECS::Entity entity) const;
        [[nodiscard]] const std::vector<ECS::Entity>& GetChildren(ECS::Entity entity) const;

        /// Combines `entity`'s own TransformComponent with every ancestor's,
        /// parent-to-child (outermost ancestor first) - matching the
        /// "parent * child" composition order Mat4's own documentation
        /// (M4) and every other transform-combining call site in this
        /// engine already uses.
        [[nodiscard]] Math::Mat4 GetWorldTransform(ECS::Entity entity) const;

        [[nodiscard]] ECS::Registry& GetRegistry() noexcept { return m_Registry; }
        [[nodiscard]] const ECS::Registry& GetRegistry() const noexcept { return m_Registry; }

        [[nodiscard]] ECS::Entity FindEntityByID(UUID id) const;

    private:
        friend class SceneSerializer; // needs CreateEntityWithID to preserve UUIDs across a load - see SceneSerializer.cpp

        /// Same defaults as CreateEntity, but with a caller-supplied UUID
        /// instead of a freshly generated one. Exists solely for
        /// SceneSerializer's deserialization path, which must recreate
        /// entities with their ORIGINAL ids, not new random ones - if
        /// deserializing assigned fresh UUIDs, every previously-saved
        /// Parent reference elsewhere in the same file would no longer
        /// resolve to anything.
        ECS::Entity CreateEntityWithID(UUID id, const std::string& name);

        [[nodiscard]] bool IsDescendantOf(ECS::Entity potentialDescendant, ECS::Entity ancestor) const;
        void RemoveFromParentsChildren(ECS::Entity entity);

        ECS::Registry m_Registry;
    };

} // namespace Engine::Scene
