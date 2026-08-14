#include "Engine/Physics/AABB.h"

#include <catch2/catch_test_macros.hpp>

using namespace Engine::Physics;
using Engine::Math::Vec3;

TEST_CASE("FromCenterHalfExtents produces the expected Min/Max", "[physics][aabb]")
{
    const AABB box = AABB::FromCenterHalfExtents(Vec3(1.0f, 2.0f, 3.0f), Vec3(0.5f, 1.0f, 1.5f));

    REQUIRE(box.Min == Vec3(0.5f, 1.0f, 1.5f));
    REQUIRE(box.Max == Vec3(1.5f, 3.0f, 4.5f));
}

TEST_CASE("GetCenter and GetHalfExtents round-trip through FromCenterHalfExtents", "[physics][aabb]")
{
    const Vec3 center(2.0f, -1.0f, 0.5f);
    const Vec3 halfExtents(1.0f, 2.0f, 3.0f);
    const AABB box = AABB::FromCenterHalfExtents(center, halfExtents);

    REQUIRE(box.GetCenter() == center);
    REQUIRE(box.GetHalfExtents() == halfExtents);
}

TEST_CASE("Overlapping boxes report Intersects", "[physics][aabb]")
{
    const AABB a = AABB::FromCenterHalfExtents(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));
    const AABB b = AABB::FromCenterHalfExtents(Vec3(1.5f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));

    REQUIRE(a.Intersects(b));
    REQUIRE(b.Intersects(a)); // symmetry - the test itself doesn't care which box calls it on which
}

TEST_CASE("Separated boxes on a single axis do not intersect", "[physics][aabb]")
{
    const AABB a = AABB::FromCenterHalfExtents(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));
    const AABB b = AABB::FromCenterHalfExtents(Vec3(10.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));

    REQUIRE_FALSE(a.Intersects(b));
}

TEST_CASE("Boxes touching exactly at a face boundary count as intersecting", "[physics][aabb]")
{
    // Box A spans [-1,1], Box B spans [1,3] - they share the plane x=1
    // exactly. Intersects() uses <=/>=, so this is deliberately inclusive.
    const AABB a = AABB::FromCenterHalfExtents(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));
    const AABB b = AABB::FromCenterHalfExtents(Vec3(2.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));

    REQUIRE(a.Intersects(b));
}

TEST_CASE("Boxes separated on only one axis, overlapping on the other two, do not intersect", "[physics][aabb]")
{
    // This is the case a buggy "overlap on ANY axis" implementation would
    // get wrong - correct AABB overlap requires overlap on ALL 3 axes
    // simultaneously.
    const AABB a = AABB::FromCenterHalfExtents(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));
    const AABB b = AABB::FromCenterHalfExtents(Vec3(0.0f, 0.0f, 10.0f), Vec3(1.0f, 1.0f, 1.0f));

    REQUIRE_FALSE(a.Intersects(b));
}
