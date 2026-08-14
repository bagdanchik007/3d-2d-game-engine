#pragma once

#include "Engine/Math/Vec3.h"

namespace Engine::Physics
{
    class RigidBody;

    /// Result of a narrow-phase collision test between two bodies.
    /// Normal points from A toward B by convention - every place that
    /// constructs or consumes a manifold (DetectCollisions, ResolveCollisions
    /// in PhysicsWorld.cpp) agrees on this direction, which is what lets
    /// resolution apply the correction/impulse to A and B with opposite
    /// signs instead of re-deriving direction from scratch.
    struct CollisionManifold
    {
        RigidBody* A = nullptr;
        RigidBody* B = nullptr;
        Math::Vec3 Normal{0.0f, 0.0f, 0.0f};
        float PenetrationDepth = 0.0f;
    };

} // namespace Engine::Physics
