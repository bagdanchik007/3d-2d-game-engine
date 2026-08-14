#pragma once

#include "Engine/Math/Vec3.h"

namespace Engine::Physics
{
    /// Axis-aligned bounding box, defined by opposite corners.
    ///
    /// For M10's box-only colliders, an AABB test IS the exact collision
    /// test (no separate "loose broad-phase bound" vs "precise shape"
    /// distinction the way an AABB-around-a-sphere would need) - see
    /// PhysicsWorld.h for why the broad/narrow phase separation is still
    /// kept structurally distinct despite that.
    struct AABB
    {
        Math::Vec3 Min;
        Math::Vec3 Max;

        [[nodiscard]] static constexpr AABB FromCenterHalfExtents(const Math::Vec3& center, const Math::Vec3& halfExtents) noexcept
        {
            return AABB{center - halfExtents, center + halfExtents};
        }

        [[nodiscard]] constexpr Math::Vec3 GetCenter() const noexcept
        {
            return (Min + Max) * 0.5f;
        }

        [[nodiscard]] constexpr Math::Vec3 GetHalfExtents() const noexcept
        {
            return (Max - Min) * 0.5f;
        }

        [[nodiscard]] constexpr bool Intersects(const AABB& other) const noexcept
        {
            // Separating-axis test on each of the 3 world axes: if the
            // boxes are separated along ANY single axis, they cannot be
            // overlapping - this is the entire SAT algorithm for two
            // axis-aligned boxes, since their only possible separating
            // axes are the 3 world axes (no arbitrary-orientation edges to
            // test, unlike OBB-vs-OBB).
            return Min.x <= other.Max.x && Max.x >= other.Min.x &&
                   Min.y <= other.Max.y && Max.y >= other.Min.y &&
                   Min.z <= other.Max.z && Max.z >= other.Min.z;
        }
    };

} // namespace Engine::Physics
