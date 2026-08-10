#include "Engine/ECS/Registry.h"

#include <catch2/catch_test_macros.hpp>

using namespace Engine::ECS;

namespace
{
    struct Health
    {
        int Current = 100;
        int Max = 100;
    };

    struct Velocity
    {
        float dx = 0.0f;
        float dy = 0.0f;
    };
}

TEST_CASE("Create returns a valid, non-null entity", "[ecs][registry]")
{
    Registry registry;
    const Entity e = registry.Create();

    REQUIRE_FALSE(e.IsNull());
    REQUIRE(registry.IsValid(e));
}

TEST_CASE("Created entities have distinct indices", "[ecs][registry]")
{
    Registry registry;
    const Entity a = registry.Create();
    const Entity b = registry.Create();

    REQUIRE(a.Index() != b.Index());
    REQUIRE_FALSE(a == b);
}

TEST_CASE("Destroy invalidates the entity", "[ecs][registry]")
{
    Registry registry;
    const Entity e = registry.Create();
    registry.Destroy(e);

    REQUIRE_FALSE(registry.IsValid(e));
}

TEST_CASE("Destroying an already-invalid entity is a safe no-op", "[ecs][registry]")
{
    Registry registry;
    const Entity e = registry.Create();
    registry.Destroy(e);

    REQUIRE_NOTHROW(registry.Destroy(e)); // double-destroy must not crash or corrupt state
    REQUIRE_FALSE(registry.IsValid(e));
}

TEST_CASE("A recycled index gets a bumped generation, invalidating the old handle", "[ecs][registry]")
{
    Registry registry;
    const Entity original = registry.Create();
    registry.Destroy(original);

    const Entity recycled = registry.Create();

    REQUIRE(recycled.Index() == original.Index()); // same slot reused
    REQUIRE(recycled.Generation() != original.Generation());
    REQUIRE_FALSE(registry.IsValid(original));
    REQUIRE(registry.IsValid(recycled));
}

TEST_CASE("AddComponent then GetComponent round-trips the value", "[ecs][registry]")
{
    Registry registry;
    const Entity e = registry.Create();

    registry.AddComponent<Health>(e, 80, 100);

    REQUIRE(registry.HasComponent<Health>(e));
    REQUIRE(registry.GetComponent<Health>(e).Current == 80);
    REQUIRE(registry.GetComponent<Health>(e).Max == 100);
}

TEST_CASE("HasComponent is false for a component type never added to any entity", "[ecs][registry]")
{
    Registry registry;
    const Entity e = registry.Create();

    REQUIRE_FALSE(registry.HasComponent<Health>(e)); // pool for Health doesn't even exist yet
}

TEST_CASE("RemoveComponent clears HasComponent without affecting other components", "[ecs][registry]")
{
    Registry registry;
    const Entity e = registry.Create();

    registry.AddComponent<Health>(e, 50, 100);
    registry.AddComponent<Velocity>(e, 1.0f, 2.0f);

    registry.RemoveComponent<Health>(e);

    REQUIRE_FALSE(registry.HasComponent<Health>(e));
    REQUIRE(registry.HasComponent<Velocity>(e)); // untouched
}

TEST_CASE("Destroy removes all of an entity's components across every pool", "[ecs][registry]")
{
    Registry registry;
    const Entity e = registry.Create();

    registry.AddComponent<Health>(e, 50, 100);
    registry.AddComponent<Velocity>(e, 1.0f, 2.0f);

    registry.Destroy(e);

    // Querying with an invalid entity should report false, not stale data.
    REQUIRE_FALSE(registry.HasComponent<Health>(e));
    REQUIRE_FALSE(registry.HasComponent<Velocity>(e));
}

TEST_CASE("An entity that gets a component's index recycled from a destroyed entity does not inherit its components", "[ecs][registry]")
{
    Registry registry;
    const Entity original = registry.Create();
    registry.AddComponent<Health>(original, 1, 100);
    registry.Destroy(original);

    const Entity recycled = registry.Create();

    // This is the ECS-level version of the same stale-handle guarantee
    // ComponentPoolTests.cpp verifies at the sparse-set level: a brand new
    // entity, even one reusing a destroyed entity's index, starts with no
    // components at all.
    REQUIRE_FALSE(registry.HasComponent<Health>(recycled));
}
