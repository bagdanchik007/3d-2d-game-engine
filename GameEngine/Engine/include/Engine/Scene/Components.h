#pragma once

#include "Engine/Core/UUID.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Math/Math.h"

#include <string>
#include <utility>
#include <vector>

namespace Engine::Scene
{
    /// A persistent identifier, separate from the entity's ECS::Entity
    /// handle. This is the entire reason SceneSerializer (see
    /// Assets/SceneSerializer.h) can save and restore parent/child
    /// relationships at all: ECS::Entity is index+generation into a
    /// specific live Registry (see ECS/Entity.h) and is meaningless once
    /// that Registry is destroyed and a new one is loaded - saving "entity
    /// 7" to disk and expecting index 7 to mean the same thing on the next
    /// run would be relying on an implementation detail that isn't even
    /// stable across two runs of the SAME program, let alone a save file
    /// meant to be reloaded later. IDComponent is added by
    /// Scene::CreateEntity to every entity, unconditionally.
    struct IDComponent
    {
        Engine::UUID ID;
    };
    /// Local (not yet hierarchical - that's M7's "Transform hierarchy")
    /// position/rotation/scale, with the composed matrix computed on
    /// demand rather than cached. Caching would mean tracking a dirty flag
    /// and invalidating it correctly from every mutation site; with no
    /// profiling evidence yet that recomputing this per-access is a
    /// bottleneck, that complexity isn't justified today.
    struct TransformComponent
    {
        Math::Vec3 Position{0.0f, 0.0f, 0.0f};
        Math::Quaternion Rotation{};
        Math::Vec3 Scale{1.0f, 1.0f, 1.0f};

        [[nodiscard]] Math::Mat4 GetLocalMatrix() const noexcept
        {
            return Math::Mat4::Translate(Position) * Rotation.ToMat4() * Math::Mat4::Scale(Scale);
        }
    };

    /// Human-readable identifier, mainly for the future editor's hierarchy
    /// panel (M8) and debug logging - never used for entity lookup, which
    /// stays Entity-handle-based throughout the engine.
    struct TagComponent
    {
        std::string Name;

        TagComponent() = default;
        explicit TagComponent(std::string name) noexcept : Name(std::move(name)) {}
    };

    /// Parent/child links for the transform hierarchy. Mutated exclusively
    /// through Scene::SetParent/DestroyEntity (see Scene.h) - never set
    /// these fields directly via registry.GetComponent<RelationshipComponent>,
    /// since Scene is what keeps a child's Parent and the parent's Children
    /// list consistent with each other and guards against creating a
    /// parenting cycle. This component intentionally exposes no invariants
    /// of its own; it's plain data whose correctness is Scene's
    /// responsibility, the same division of labor Entity.h describes
    /// between an Entity value and the Registry that alone may construct
    /// a valid one.
    struct RelationshipComponent
    {
        ECS::Entity Parent = ECS::NullEntity;
        std::vector<ECS::Entity> Children;
    };

} // namespace Engine::Scene
