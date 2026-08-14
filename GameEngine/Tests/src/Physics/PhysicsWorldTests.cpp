#include "Engine/Physics/PhysicsWorld.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Engine::Physics;
using Engine::Math::Vec3;
using Catch::Approx;

TEST_CASE("Update accumulates leftover time across calls instead of dropping it", "[physics][world]")
{
    PhysicsWorld world;
    world.SetGravity(Vec3(0.0f, -10.0f, 0.0f));

    RigidBodyDef def;
    def.Position = Vec3(0.0f, 100.0f, 0.0f);
    RigidBody* body = world.CreateBody(def);

    const float halfStep = PhysicsWorld::FixedTimestep * 0.5f;
    world.Update(halfStep);
    REQUIRE(body->GetVelocity().y == Approx(0.0f));

    world.Update(halfStep);
    REQUIRE(body->GetVelocity().y != Approx(0.0f));
}

TEST_CASE("An extreme deltaTime does not spiral into unbounded catch-up steps", "[physics][world]")
{
    PhysicsWorld world;

    RigidBodyDef def;
    def.Position = Vec3(0.0f, 1000.0f, 0.0f);
    RigidBody* body = world.CreateBody(def);

    world.Update(10.0f);

    REQUIRE_FALSE(std::isnan(body->GetPosition().y));
}

TEST_CASE("Two dynamic bodies with zero restitution come to rest without bouncing", "[physics][world]")
{
    PhysicsWorld world;
    world.SetGravity(Vec3(0.0f, -10.0f, 0.0f));

    RigidBodyDef floorDef;
    floorDef.Position = Vec3(0.0f, 0.0f, 0.0f);
    floorDef.HalfExtents = Vec3(10.0f, 0.5f, 10.0f);
    floorDef.IsStatic = true;
    floorDef.Restitution = 0.0f;
    world.CreateBody(floorDef);

    RigidBodyDef boxDef;
    boxDef.Position = Vec3(0.0f, 2.0f, 0.0f);
    boxDef.HalfExtents = Vec3(0.5f, 0.5f, 0.5f);
    boxDef.Restitution = 0.0f;
    RigidBody* box = world.CreateBody(boxDef);

    for (int i = 0; i < 300; ++i)
        world.Update(PhysicsWorld::FixedTimestep);

    REQUIRE(box->GetPosition().y == Approx(1.0f).margin(0.05f));
    REQUIRE(std::fabs(box->GetVelocity().y) < 0.5f);
}
