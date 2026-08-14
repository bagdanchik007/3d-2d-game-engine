#pragma once

#include "Engine/Physics/AABB.h"

namespace Engine::Physics
{
    /// Describes a body at creation time - the same "plain descriptor
    /// struct" pattern as WindowProps (M3) and FramebufferSpecification
    /// (M8), used here so PhysicsWorld::CreateBody's signature doesn't
    /// grow an unreadable list of positional float/bool parameters as
    /// more properties (friction, restitution, ...) were added during
    /// design.
    struct RigidBodyDef
    {
        Math::Vec3 Position{0.0f, 0.0f, 0.0f};
        Math::Vec3 HalfExtents{0.5f, 0.5f, 0.5f};
        float Mass = 1.0f;
        float Restitution = 0.3f; // 0 = fully inelastic (no bounce), 1 = perfectly elastic
        float Friction = 0.5f;

        /// A static body has infinite mass (never moves, never affected by
        /// forces or collision impulses) but still participates in
        /// collision - the standard way to represent a ground plane or
        /// wall without every dynamic-body calculation needing a special
        /// case for "what if the other body can't move".
        bool IsStatic = false;
    };

    class RigidBody
    {
    public:
        explicit RigidBody(const RigidBodyDef& def) noexcept
            : m_Position(def.Position)
            , m_HalfExtents(def.HalfExtents)
            , m_Restitution(def.Restitution)
            , m_Friction(def.Friction)
            , m_IsStatic(def.IsStatic)
            // A static body's inverse mass is exactly 0, not "a very large
            // mass": every impulse/positional-correction formula in
            // PhysicsWorld.cpp multiplies by inverse mass, and 0 is what
            // makes those formulas naturally apply zero effect to a static
            // body without an explicit `if (IsStatic)` branch scattered
            // through the solver. This is the standard rigid-body-physics
            // trick for representing "infinite mass" cheaply and exactly,
            // not an approximation.
            , m_InverseMass(def.IsStatic || def.Mass <= 0.0f ? 0.0f : 1.0f / def.Mass)
        {
        }

        [[nodiscard]] const Math::Vec3& GetPosition() const noexcept { return m_Position; }
        void SetPosition(const Math::Vec3& position) noexcept { m_Position = position; }

        [[nodiscard]] const Math::Vec3& GetVelocity() const noexcept { return m_Velocity; }
        void SetVelocity(const Math::Vec3& velocity) noexcept { m_Velocity = velocity; }

        [[nodiscard]] const Math::Vec3& GetHalfExtents() const noexcept { return m_HalfExtents; }
        [[nodiscard]] float GetInverseMass() const noexcept { return m_InverseMass; }
        [[nodiscard]] float GetRestitution() const noexcept { return m_Restitution; }
        [[nodiscard]] float GetFriction() const noexcept { return m_Friction; }
        [[nodiscard]] bool IsStatic() const noexcept { return m_IsStatic; }

        [[nodiscard]] AABB GetAABB() const noexcept
        {
            return AABB::FromCenterHalfExtents(m_Position, m_HalfExtents);
        }

    private:
        Math::Vec3 m_Position;
        Math::Vec3 m_Velocity{0.0f, 0.0f, 0.0f};
        Math::Vec3 m_HalfExtents;
        float m_Restitution;
        float m_Friction;
        bool m_IsStatic;
        float m_InverseMass;
    };

} // namespace Engine::Physics
