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
    world.SetGravity(Vec3(0.0f, -10.0f, 0.0f)); // round number, easier to hand-verify than -9.81

    RigidBodyDef def;
    def.Position = Vec3(0.0f, 100.0f, 0.0f); // high enough that it never reaches the ground during this test
    RigidBody* body = world.CreateBody(def);

    const float dt = PhysicsWorld::FixedTimestep;
    world.Update(dt);

    // Semi-implicit Euler for one step: v1 = v0 + g*dt = -10*dt;
    // p1 = p0 + v1*dt = 100 - 10*dt*dt. Checking the EXACT semi-implicit
    // formula (not explicit Euler's p0 + v0*dt, which would just be 100
    // here since v0 starts at 0) is what would catch an accidental
    // integration-order swap.
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
    {
        world.Update(PhysicsWorld::FixedTimestep);
    }

    REQUIRE(body->GetPosition().y == Approx(5.0f));
    REQUIRE(body->GetVelocity().y == Approx(0.0f));
}

TEST_CASE("Update accumulates leftover time across calls instead of dropping it", "[physics][world]")
