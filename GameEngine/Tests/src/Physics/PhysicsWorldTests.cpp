#include "Engine/Physics/PhysicsWorld.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Engine::Physics;
using Engine::Math::Vec3;
using Catch::Approx;

TEST_CASE("A body falls under gravity via semi-implicit Euler integration", "[physics][world]")
{
    PhysicsWorld world;
    world.SetGravity(Vec3(0.0f, -10.0f, 0.0f));

    RigidBodyDef def;
    def.Position = Vec3(0.0f, 100.0f, 0.0f);
    RigidBody* body = world.CreateBody(def);

    const float dt = PhysicsWorld::FixedTimestep;
    world.Update(dt);

    const float expectedVelocityY = -10.0f * dt;
    const float expectedPositionY = 100.0f + expectedVelocityY * dt;

    REQUIRE(body->GetVelocity().y == Approx(expectedVelocityY));
    REQUIRE(body->GetPosition().y == Approx(expectedPositionY));
}

TEST_CASE("A static body is unaffected by gravity", "[physics][world]")
{
    PhysicsWorld world;
    world.SetGravity(Vec3(0.0f, -10.0f, 0.0f));

    RigidBodyDef def;
    def.Position = Vec3(0.0f, 5.0f, 0.0f);
    def.IsStatic = true;
    RigidBody* body = world.CreateBody(def);

    for (int i = 0; i < 60; ++i)
        world.Update(PhysicsWorld::FixedTimestep);

    REQUIRE(body->GetPosition().y == Approx(5.0f));
    REQUIRE(body->GetVelocity().y == Approx(0.0f));
}

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

TEST_CASE("A body with high restitution bounces - upward velocity after impact", "[physics][world]")
{
    PhysicsWorld world;
    world.SetGravity(Vec3(0.0f, -10.0f, 0.0f));

    RigidBodyDef floorDef;
    floorDef.Position = Vec3(0.0f, 0.0f, 0.0f);
    floorDef.HalfExtents = Vec3(10.0f, 0.5f, 10.0f);
    floorDef.IsStatic = true;
    floorDef.Restitution = 0.9f;
    world.CreateBody(floorDef);

    RigidBodyDef ballDef;
    ballDef.Position = Vec3(0.0f, 3.0f, 0.0f);
    ballDef.HalfExtents = Vec3(0.3f, 0.3f, 0.3f);
    ballDef.Restitution = 0.9f;
    RigidBody* ball = world.CreateBody(ballDef);

    bool observedUpwardVelocityAfterFalling = false;
    bool wasFalling = false;
    for (int i = 0; i < 200; ++i)
    {
        world.Update(PhysicsWorld::FixedTimestep);

        if (ball->GetVelocity().y < -0.1f)
            wasFalling = true;
        else if (wasFalling && ball->GetVelocity().y > 0.1f)
        {
            observedUpwardVelocityAfterFalling = true;
            break;
        }
    }

    REQUIRE(observedUpwardVelocityAfterFalling);
}
