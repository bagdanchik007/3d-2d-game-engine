#include "Engine/Physics/PhysicsWorld.h"

#include "Engine/Math/MathUtils.h"

#include <algorithm>

namespace Engine::Physics
{
    RigidBody* PhysicsWorld::CreateBody(const RigidBodyDef& def)
    {
        m_Bodies.push_back(std::make_unique<RigidBody>(def));
        return m_Bodies.back().get();
    }

    void PhysicsWorld::RemoveBody(const RigidBody* body)
    {
        const auto it = std::find_if(m_Bodies.begin(), m_Bodies.end(),
            [body](const std::unique_ptr<RigidBody>& candidate) { return candidate.get() == body; });

        if (it != m_Bodies.end())
        {
            m_Bodies.erase(it);
        }
    }

    void PhysicsWorld::Update(float deltaTime)
    {
        m_Accumulator += deltaTime;

        int stepsTaken = 0;
        while (m_Accumulator >= FixedTimestep && stepsTaken < kMaxStepsPerUpdate)
        {
            Step(FixedTimestep);
            m_Accumulator -= FixedTimestep;
            ++stepsTaken;
        }

        // If the step cap was hit, the remaining accumulated time is
        // deliberately dropped rather than left to balloon indefinitely -
        // see the "spiral of death" note in PhysicsWorld.h. Falling behind
        // real time by a fraction of a second after an extreme stall is an
        // acceptable trade against the alternative of that stall getting
        // permanently worse.
        if (stepsTaken == kMaxStepsPerUpdate)
        {
            m_Accumulator = 0.0f;
        }
    }

    void PhysicsWorld::Step(float fixedDt)
    {
        IntegrateForces(fixedDt);
        const std::vector<CollisionManifold> manifolds = DetectCollisions();
        ResolveCollisions(manifolds);
    }

    void PhysicsWorld::IntegrateForces(float fixedDt) const
    {
        for (const auto& body : m_Bodies)
        {
            if (body->IsStatic())
            {
                continue;
            }

            // Semi-implicit (symplectic) Euler: update velocity FIRST,
            // then use the NEW velocity to update position - not explicit
            // Euler, which would use the old velocity for the position
            // update. Semi-implicit Euler is unconditionally stable for
            // this kind of simple damped/undamped motion at a fixed
            // timestep in every case this engine exercises, where plain
            // explicit Euler is prone to energy gain (visibly accelerating
            // orbits/oscillations over time) at the same timestep - a
            // well-known, textbook reason to prefer it, not a subtle
            // micro-optimization.
            Math::Vec3 velocity = body->GetVelocity();
            velocity += m_Gravity * fixedDt;
            body->SetVelocity(velocity);

            Math::Vec3 position = body->GetPosition();
            position += velocity * fixedDt;
            body->SetPosition(position);
        }
    }

    std::vector<CollisionManifold> PhysicsWorld::DetectCollisions() const
    {
        std::vector<CollisionManifold> manifolds;

        // Naive O(n^2) all-pairs test - see the class-level comment in
        // PhysicsWorld.h for why a spatial-partitioning broad phase isn't
        // implemented here.
        for (std::size_t i = 0; i < m_Bodies.size(); ++i)
        {
            for (std::size_t j = i + 1; j < m_Bodies.size(); ++j)
            {
                RigidBody* a = m_Bodies[i].get();
                RigidBody* b = m_Bodies[j].get();

                if (a->IsStatic() && b->IsStatic())
                {
                    continue; // two immovable bodies have nothing to resolve between them, ever
                }

                const AABB aabbA = a->GetAABB();
                const AABB aabbB = b->GetAABB();
                if (!aabbA.Intersects(aabbB))
                {
                    continue;
                }

                // Minimum-penetration-axis test: compute the overlap on
                // each of the 3 axes (guaranteed positive here, since
                // Intersects() already confirmed overlap on all 3), and
                // treat the axis with the SMALLEST overlap as the
                // collision normal's axis - that is the axis along which
                // the two boxes are "least stuck into each other", and
                // therefore the direction that separates them with the
                // least movement. This is the standard AABB-vs-AABB
                // manifold construction, not an approximation specific to
                // this engine.
                const Math::Vec3 centerA = aabbA.GetCenter();
                const Math::Vec3 centerB = aabbB.GetCenter();

                const float overlapX = std::min(aabbA.Max.x, aabbB.Max.x) - std::max(aabbA.Min.x, aabbB.Min.x);
                const float overlapY = std::min(aabbA.Max.y, aabbB.Max.y) - std::max(aabbA.Min.y, aabbB.Min.y);
                const float overlapZ = std::min(aabbA.Max.z, aabbB.Max.z) - std::max(aabbA.Min.z, aabbB.Min.z);

                CollisionManifold manifold;
                manifold.A = a;
                manifold.B = b;

                if (overlapX <= overlapY && overlapX <= overlapZ)
                {
                    manifold.PenetrationDepth = overlapX;
                    manifold.Normal = Math::Vec3((centerB.x >= centerA.x) ? 1.0f : -1.0f, 0.0f, 0.0f);
                }
                else if (overlapY <= overlapX && overlapY <= overlapZ)
                {
                    manifold.PenetrationDepth = overlapY;
                    manifold.Normal = Math::Vec3(0.0f, (centerB.y >= centerA.y) ? 1.0f : -1.0f, 0.0f);
                }
                else
                {
                    manifold.PenetrationDepth = overlapZ;
                    manifold.Normal = Math::Vec3(0.0f, 0.0f, (centerB.z >= centerA.z) ? 1.0f : -1.0f);
                }

                manifolds.push_back(manifold);
            }
        }

        return manifolds;
    }

    void PhysicsWorld::ResolveCollisions(const std::vector<CollisionManifold>& manifolds)
    {
        for (const CollisionManifold& manifold : manifolds)
        {
            RigidBody& a = *manifold.A;
            RigidBody& b = *manifold.B;

            const float invMassSum = a.GetInverseMass() + b.GetInverseMass();
            if (invMassSum <= 0.0f)
            {
                continue; // both effectively static (defensive - DetectCollisions already skips static-static pairs)
            }

            // --- Positional correction ---------------------------------
            // Pushes the two bodies apart along the collision normal,
            // proportional to each body's share of the combined inverse
            // mass, so a light body is displaced more than a heavy one.
            // kSlop allows a small amount of interpenetration before any
            // correction kicks in, and kPercent corrects only a fraction
            // of the remaining penetration per step rather than all of it
            // at once - both are standard mitigations against jitter: a
            // full, immediate correction every step tends to overshoot
            // and oscillate for stacked or resting contacts.
            constexpr float kSlop = 0.01f;
            constexpr float kPercent = 0.2f;
            const float correctionMagnitude = std::max(manifold.PenetrationDepth - kSlop, 0.0f) / invMassSum * kPercent;
            const Math::Vec3 correction = manifold.Normal * correctionMagnitude;
            a.SetPosition(a.GetPosition() - correction * a.GetInverseMass());
            b.SetPosition(b.GetPosition() + correction * b.GetInverseMass());

            // --- Normal impulse (restitution) --------------------------
            const Math::Vec3 relativeVelocity = b.GetVelocity() - a.GetVelocity();
            const float velocityAlongNormal = Math::Dot(relativeVelocity, manifold.Normal);

            if (velocityAlongNormal > 0.0f)
            {
                continue; // Already separating (common for a resting/overlapping contact resolved in a previous step) - applying an impulse here would push them together instead of apart.
            }

            // Combined restitution uses the MINIMUM of the two bodies'
            // values, not their average: a perfectly bouncy ball resting
            // on a totally inelastic floor should not bounce at all, and
            // min() enforces exactly that, whereas averaging would
            // incorrectly let some bounce through. This matches the
            // combination rule used by Box2D and most other real-time
            // solvers, not an engine-specific choice.
            const float restitution = std::min(a.GetRestitution(), b.GetRestitution());
            const float normalImpulseMagnitude = -(1.0f + restitution) * velocityAlongNormal / invMassSum;
            const Math::Vec3 normalImpulse = manifold.Normal * normalImpulseMagnitude;

            a.SetVelocity(a.GetVelocity() - normalImpulse * a.GetInverseMass());
            b.SetVelocity(b.GetVelocity() + normalImpulse * b.GetInverseMass());

            // --- Friction (Coulomb, clamped to the friction cone) ------
            // Recomputes relative velocity AFTER the normal impulse above
            // has already been applied - a sequential-impulse solver step,
            // not a single simultaneous solve. This is a simplification
            // (a proper simultaneous solve would jointly consider normal
            // and tangential impulses), but is the standard approach used
            // by real-time solvers like Box2D specifically because it
            // converges in one pass for the vast majority of everyday
            // contacts, at a fraction of the cost of an iterative
            // simultaneous solver.
            const Math::Vec3 relativeVelocityAfterNormal = b.GetVelocity() - a.GetVelocity();
            Math::Vec3 tangent = relativeVelocityAfterNormal - manifold.Normal * Math::Dot(relativeVelocityAfterNormal, manifold.Normal);

            constexpr float kTangentEpsilon = 1e-6f;
            if (tangent.LengthSquared() > kTangentEpsilon)
            {
                tangent = tangent.Normalized();

                const float tangentImpulseMagnitude = -Math::Dot(relativeVelocityAfterNormal, tangent) / invMassSum;

                // Combined friction coefficient uses the AVERAGE of the
                // two bodies' values - unlike restitution's min() rule
                // above, there is no single universally "correct" way to
                // combine two friction coefficients (real materials don't
                // compose this simply either), and averaging is the
                // common, documented simplification most real-time
                // engines default to when a full per-material-pair
                // friction table isn't worth the complexity.
                const float combinedFriction = (a.GetFriction() + b.GetFriction()) * 0.5f;

                // Coulomb's law: the tangential (friction) force can never
                // exceed the normal force times the friction coefficient -
                // clamping the tangent impulse to +-mu*normalImpulse is
                // that law applied directly to impulses instead of forces.
                const float maxFrictionImpulse = combinedFriction * normalImpulseMagnitude;
                const float clampedTangentImpulse = Math::Clamp(tangentImpulseMagnitude, -maxFrictionImpulse, maxFrictionImpulse);

                const Math::Vec3 frictionImpulse = tangent * clampedTangentImpulse;
                a.SetVelocity(a.GetVelocity() - frictionImpulse * a.GetInverseMass());
                b.SetVelocity(b.GetVelocity() + frictionImpulse * b.GetInverseMass());
            }
        }
    }

} // namespace Engine::Physics
