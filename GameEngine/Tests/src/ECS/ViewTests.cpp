#include "Engine/ECS/Registry.h"
#include "Engine/ECS/View.h"

#include <catch2/catch_test_macros.hpp>

#include <set>
#include <string>

using namespace Engine::ECS;

namespace
{
    struct Position
    {
        float x = 0.0f;
        float y = 0.0f;
    };

    struct Velocity
    {
        float dx = 0.0f;
        float dy = 0.0f;
    };

    struct Tag
    {
        std::string Name;
    };
}

TEST_CASE("Single-component View visits every entity with that component", "[ecs][view]")
{
    Registry registry;
    const Entity a = registry.Create();
    const Entity b = registry.Create();
    const Entity c = registry.Create(); // no Position - must not be visited

    registry.AddComponent<Position>(a, 1.0f, 1.0f);
    registry.AddComponent<Position>(b, 2.0f, 2.0f);
    (void)c;

    std::set<EntityIndex> visited;
    View<Position> view(registry);
    view.Each([&visited](Entity e, Position&) { visited.insert(e.Index()); });

    REQUIRE(visited.size() == 2);
    REQUIRE(visited.count(a.Index()) == 1);
    REQUIRE(visited.count(b.Index()) == 1);
    REQUIRE(visited.count(c.Index()) == 0);
}

TEST_CASE("Multi-component View only visits entities having ALL requested components", "[ecs][view]")
{
    Registry registry;
    const Entity both = registry.Create();
    const Entity positionOnly = registry.Create();
    const Entity velocityOnly = registry.Create();

    registry.AddComponent<Position>(both, 0.0f, 0.0f);
    registry.AddComponent<Velocity>(both, 1.0f, 1.0f);

    registry.AddComponent<Position>(positionOnly, 5.0f, 5.0f);
    registry.AddComponent<Velocity>(velocityOnly, 9.0f, 9.0f);

    std::set<EntityIndex> visited;
    View<Position, Velocity> view(registry);
    view.Each([&visited](Entity e, Position&, Velocity&) { visited.insert(e.Index()); });

    REQUIRE(visited.size() == 1);
    REQUIRE(visited.count(both.Index()) == 1);
}

TEST_CASE("View correctly drives iteration from the smaller pool regardless of which type it is", "[ecs][view]")
{
    Registry registry;

    // Many entities get Position (the "large" pool)...
    constexpr int kManyCount = 50;
    Entity theOneWithVelocity = NullEntity;
    for (int i = 0; i < kManyCount; ++i)
    {
        const Entity e = registry.Create();
        registry.AddComponent<Position>(e, static_cast<float>(i), 0.0f);
        if (i == kManyCount / 2)
        {
            registry.AddComponent<Velocity>(e, 1.0f, 1.0f); // ...but only one gets Velocity (the "small" pool)
            theOneWithVelocity = e;
        }
    }

    int visitCount = 0;
    Entity visitedEntity = NullEntity;
    View<Velocity, Position> view(registry); // Velocity listed first, but Position is the larger pool
    view.Each([&](Entity e, Velocity&, Position&)
    {
        ++visitCount;
        visitedEntity = e;
    });

    REQUIRE(visitCount == 1);
    REQUIRE(visitedEntity == theOneWithVelocity);
}

TEST_CASE("View mutations through Each are visible via GetComponent afterward", "[ecs][view]")
{
    Registry registry;
    const Entity e = registry.Create();
    registry.AddComponent<Position>(e, 0.0f, 0.0f);

    View<Position> view(registry);
    view.Each([](Entity, Position& pos)
    {
        pos.x = 42.0f;
        pos.y = 24.0f;
    });

    REQUIRE(registry.GetComponent<Position>(e).x == 42.0f);
    REQUIRE(registry.GetComponent<Position>(e).y == 24.0f);
}

TEST_CASE("View over a component type with no pool yet visits nothing", "[ecs][view]")
{
    Registry registry;
    const Entity e = registry.Create();
    registry.AddComponent<Position>(e, 1.0f, 1.0f);

    int visitCount = 0;
    View<Position, Tag> view(registry); // Tag pool was never created
    view.Each([&visitCount](Entity, Position&, Tag&) { ++visitCount; });

    REQUIRE(visitCount == 0);
}
