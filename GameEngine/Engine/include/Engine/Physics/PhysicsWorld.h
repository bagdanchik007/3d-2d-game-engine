#pragma once

#include "Engine/Physics/CollisionManifold.h"
#include "Engine/Physics/RigidBody.h"

#include <memory>
#include <vector>

namespace Engine::Physics
{
    /// Owns all rigid bodies and advances the simulation.
    ///
    /// Broad and narrow phase are kept as structurally separate steps
    /// (DetectCollisions does both today) even though, for box-only
    /// colliders, an AABB overlap test is already the exact collision
    /// test - there is no "loose bound vs. precise shape" gap to exploit
    /// yet. The separation is worth keeping anyway: the moment a second
    /// shape type exists (a sphere collider, say), broad phase becomes
    /// "which AABB pairs might overlap" and narrow phase becomes "run the
    /// shape-specific test for that pair" - two genuinely different
    /// algorithms sharing one interface point, not a distinction invented
    /// for its own sake.
    class PhysicsWorld
    {
    public:
        PhysicsWorld() = default;

        PhysicsWorld(const PhysicsWorld&) = delete;
        PhysicsWorld& operator=(const PhysicsWorld&) = delete;

        /// Returns a non-owning observer pointer - same ownership pattern
        /// as LayerStack::PushLayer (M2): PhysicsWorld stores bodies via
        /// unique_ptr, the caller gets a raw pointer valid until
        /// RemoveBody() or the PhysicsWorld itself is destroyed.
        RigidBody* CreateBody(const RigidBodyDef& def);
        void RemoveBody(const RigidBody* body);

        void SetGravity(const Math::Vec3& gravity) noexcept { m_Gravity = gravity; }
        [[nodiscard]] const Math::Vec3& GetGravity() const noexcept { return m_Gravity; }

        /// Advances the simulation by `deltaTime` (a variable, real
        /// frame-to-frame duration) using a fixed internal timestep,
        /// accumulating leftover time across calls so the simulation
        /// itself is deterministic regardless of the caller's frame rate -
        /// the standard "Fix Your Timestep" pattern. Capped at
        /// kMaxStepsPerUpdate fixed steps per call: without a cap, a long
        /// pause (a breakpoint, a dropped frame from window resize)
        /// would hand this function a huge deltaTime, which would then
        /// try to "catch up" with hundreds of fixed steps, taking even
        /// longer to compute than it needs to simulate - the classic
        /// "spiral of death" this guard exists specifically to prevent.
        /// The simulation falls behind real time in that scenario instead
        /// of freezing the process entirely, which is the better failure
        /// mode of the two.
        void Update(float deltaTime);

        [[nodiscard]] std::size_t GetBodyCount() const noexcept { return m_Bodies.size(); }

        static constexpr float FixedTimestep = 1.0f / 60.0f;

    private:
        void Step(float fixedDt);
        void IntegrateForces(float fixedDt) const;
        [[nodiscard]] std::vector<CollisionManifold> DetectCollisions() const;
        static void ResolveCollisions(const std::vector<CollisionManifold>& manifolds);

        std::vector<std::unique_ptr<RigidBody>> m_Bodies;
        Math::Vec3 m_Gravity{0.0f, -9.81f, 0.0f};
        float m_Accumulator = 0.0f;

        static constexpr int kMaxStepsPerUpdate = 5;
    };

} // namespace Engine::Physics
