#include "Engine/ECS/ComponentPool.h"
#include "Engine/ECS/Registry.h"

#include <catch2/catch_test_macros.hpp>

using namespace Engine::ECS;

namespace
{
    struct Position
    {
        float x = 0.0f;
        float y = 0.0f;

        bool operator==(const Position&) const = default;
    };
}

TEST_CASE("Emplace then Contains/Get round-trips the component", "[ecs][componentpool]")
{
    Registry registry;
    const Entity e = registry.Create();

    ComponentPool<Position> pool;
    pool.Emplace(e, 1.0f, 2.0f);

    REQUIRE(pool.Contains(e));
    REQUIRE(pool.Get(e) == Position{1.0f, 2.0f});
    REQUIRE(pool.Size() == 1);
}

TEST_CASE("Remove makes Contains false and shrinks Size", "[ecs][componentpool]")
{
    Registry registry;
    const Entity e = registry.Create();

    ComponentPool<Position> pool;
    pool.Emplace(e, 1.0f, 2.0f);
    pool.Remove(e);

    REQUIRE_FALSE(pool.Contains(e));
    REQUIRE(pool.Size() == 0);
}

TEST_CASE("Removing a non-member entity is a safe no-op", "[ecs][componentpool]")
{
    Registry registry;
    const Entity e = registry.Create();

    ComponentPool<Position> pool;
    pool.Remove(e); // never inserted

    REQUIRE(pool.Size() == 0);
}

TEST_CASE("Swap-remove relocates the last element into the removed slot without corrupting it", "[ecs][componentpool]")
{
    Registry registry;
    const Entity a = registry.Create();
    const Entity b = registry.Create();
    const Entity c = registry.Create();

    ComponentPool<Position> pool;
    pool.Emplace(a, 1.0f, 1.0f);
    pool.Emplace(b, 2.0f, 2.0f);
    pool.Emplace(c, 3.0f, 3.0f); // c is last in the dense array

    pool.Remove(a); // swap-remove: c moves into a's old slot

    REQUIRE_FALSE(pool.Contains(a));
    REQUIRE(pool.Contains(b));
    REQUIRE(pool.Contains(c));
    REQUIRE(pool.Get(b) == Position{2.0f, 2.0f});
    REQUIRE(pool.Get(c) == Position{3.0f, 3.0f}); // must still read correctly after being relocated
    REQUIRE(pool.Size() == 2);
}

TEST_CASE("A stale Entity handle from before index reuse does not alias the new entity's component", "[ecs][componentpool]")
{
    Registry registry;
    const Entity original = registry.Create();

    ComponentPool<Position> pool;
    pool.Emplace(original, 5.0f, 5.0f);

    registry.Destroy(original);
    pool.Remove(original); // Registry doesn't own this standalone pool in this test, so remove explicitly

    const Entity recycled = registry.Create(); // very likely reuses `original`'s index with a bumped generation
    pool.Emplace(recycled, 9.0f, 9.0f);

    // The critical assertion: querying with the OLD handle must not report
    // the NEW entity's component, even though both handles may share the
    // same underlying index.
    REQUIRE_FALSE(pool.Contains(original));
    REQUIRE(pool.Contains(recycled));
    REQUIRE(pool.Get(recycled) == Position{9.0f, 9.0f});
}

TEST_CASE("GetEntities returns a dense span matching Size", "[ecs][componentpool]")
{
    Registry registry;
    const Entity a = registry.Create();
    const Entity b = registry.Create();

    ComponentPool<Position> pool;
    pool.Emplace(a, 1.0f, 0.0f);
    pool.Emplace(b, 2.0f, 0.0f);

    const auto entities = pool.GetEntities();
    REQUIRE(entities.size() == 2);
    REQUIRE((entities[0] == a || entities[0] == b));
    REQUIRE((entities[1] == a || entities[1] == b));
    REQUIRE(entities[0] != entities[1]);
}
