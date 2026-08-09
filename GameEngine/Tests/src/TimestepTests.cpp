#include "Engine/Core/Timestep.h"

#include <catch2/catch_test_macros.hpp>

using namespace Engine;

TEST_CASE("Timestep converts seconds to milliseconds correctly", "[timestep]")
{
    const Timestep ts(0.5f);

    REQUIRE(ts.GetSeconds() == 0.5f);
    REQUIRE(ts.GetMilliseconds() == 500.0f);
}

TEST_CASE("Timestep implicitly converts to float as seconds", "[timestep]")
{
    const Timestep ts(0.25f);
    const float raw = ts;

    REQUIRE(raw == 0.25f);
}

TEST_CASE("Default-constructed Timestep is zero", "[timestep]")
{
    const Timestep ts;

    REQUIRE(ts.GetSeconds() == 0.0f);
}
