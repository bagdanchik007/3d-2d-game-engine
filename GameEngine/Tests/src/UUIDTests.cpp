#include "Engine/Core/UUID.h"

#include <catch2/catch_test_macros.hpp>

#include <unordered_set>

using namespace Engine;

TEST_CASE("Default-constructed UUIDs are extremely unlikely to collide", "[core][uuid]")
{
    // Not a proof of uniqueness (impossible to prove for randomness), but
    // a real regression guard: an accidental "return UUID(0)" or a badly
    // seeded generator would fail this near-instantly, whereas true random
    // 64-bit collisions across a few thousand draws are effectively
    // impossible (birthday-bound odds are astronomically small at this
    // sample size relative to 2^64).
    std::unordered_set<uint64_t> seen;
    constexpr int kSampleCount = 10000;
    for (int i = 0; i < kSampleCount; ++i)
    {
        const UUID id;
        seen.insert(id.Value());
    }

    REQUIRE(seen.size() == kSampleCount);
}

TEST_CASE("A UUID constructed from an explicit value round-trips exactly", "[core][uuid]")
{
    const UUID id(0x1234'5678'9abc'def0ULL);
    REQUIRE(id.Value() == 0x1234'5678'9abc'def0ULL);
}

TEST_CASE("UUIDs with the same value compare equal", "[core][uuid]")
{
    const UUID a(42);
    const UUID b(42);
    REQUIRE(a == b);
}

TEST_CASE("UUID is usable as an unordered_map/unordered_set key", "[core][uuid]")
{
    std::unordered_set<UUID> ids;
    const UUID a(1);
    const UUID b(2);
    ids.insert(a);
    ids.insert(b);

    REQUIRE(ids.size() == 2);
    REQUIRE(ids.contains(a));
    REQUIRE(ids.contains(UUID(1))); // a different UUID object, same value - must still hash/compare equal
}
