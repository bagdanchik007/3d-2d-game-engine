#include "Engine/Physics/RigidBody.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Engine::Physics;
using Engine::Math::Vec3;
using Catch::Approx;

TEST_CASE("A dynamic body's inverse mass is 1/mass", "[physics][rigidbody]")
{
    RigidBodyDef def;
    def.Mass = 2.0f;
    const RigidBody body(def);

    REQUIRE(body.GetInverseMass() == Approx(0.5f));
    REQUIRE_FALSE(body.IsStatic());
}

TEST_CASE("A static body's inverse mass is exactly 0 regardless of the Mass field", "[physics][rigidbody]")
{
    RigidBodyDef def;
    def.Mass = 100.0f; // deliberately non-default, to confirm IsStatic overrides it rather than being ignored
    def.IsStatic = true;
    const RigidBody body(def);

    REQUIRE(body.GetInverseMass() == 0.0f);
    REQUIRE(body.IsStatic());
}

TEST_CASE("A zero or negative mass produces an inverse mass of 0, not a division by zero", "[physics][rigidbody]")
{
    RigidBodyDef def;
    def.Mass = 0.0f;
    const RigidBody body(def);

    REQUIRE(body.GetInverseMass() == 0.0f);
    REQUIRE_FALSE(std::isnan(body.GetInverseMass()));
}

TEST_CASE("GetAABB reflects the body's current position and half-extents", "[physics][rigidbody]")
{
    RigidBodyDef def;
    def.Position = Vec3(5.0f, 0.0f, 0.0f);
    def.HalfExtents = Vec3(1.0f, 2.0f, 3.0f);
    RigidBody body(def);

    const AABB box = body.GetAABB();
    REQUIRE(box.GetCenter() == Vec3(5.0f, 0.0f, 0.0f));
    REQUIRE(box.GetHalfExtents() == Vec3(1.0f, 2.0f, 3.0f));

    body.SetPosition(Vec3(10.0f, 0.0f, 0.0f));
    REQUIRE(body.GetAABB().GetCenter() == Vec3(10.0f, 0.0f, 0.0f));
}
