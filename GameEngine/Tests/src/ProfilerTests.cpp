#include "Engine/Core/Profiler.h"

#include <catch2/catch_test_macros.hpp>

#include <thread>

using namespace Engine;

TEST_CASE("BeginFrame clears any previously submitted results", "[core][profiler]")
{
    Profiler::BeginFrame();
    Profiler::Submit("Section", 1.0f);
    REQUIRE(Profiler::GetResults().size() == 1);

    Profiler::BeginFrame();
    REQUIRE(Profiler::GetResults().empty());
}

TEST_CASE("Submit appends a named result", "[core][profiler]")
{
    Profiler::BeginFrame();
    Profiler::Submit("Physics::Update", 2.5f);
    Profiler::Submit("Renderer2D::Flush", 0.8f);

    const auto& results = Profiler::GetResults();
    REQUIRE(results.size() == 2);
    REQUIRE(results[0].Name == "Physics::Update");
    REQUIRE(results[0].DurationMs == 2.5f);
    REQUIRE(results[1].Name == "Renderer2D::Flush");
}

TEST_CASE("SetLastFrameTimeMs/GetLastFrameTimeMs round-trips", "[core][profiler]")
{
    Profiler::SetLastFrameTimeMs(16.67f);
    REQUIRE(Profiler::GetLastFrameTimeMs() == 16.67f);
}

TEST_CASE("ScopedTimer submits a result to Profiler on destruction, not on construction", "[core][profiler]")
{
    Profiler::BeginFrame();

    {
        const ScopedTimer timer("ScopedSection");
        // Still inside the scope: the timer hasn't been destroyed yet, so
        // nothing should have been submitted - this is what actually
        // verifies the RAII destructor is doing the submission, rather
        // than the constructor eagerly submitting a (necessarily zero)
        // duration immediately.
        REQUIRE(Profiler::GetResults().empty());
    }

    REQUIRE(Profiler::GetResults().size() == 1);
    REQUIRE(Profiler::GetResults()[0].Name == "ScopedSection");
}

TEST_CASE("ScopedTimer records a non-negative duration approximately matching its actual lifetime", "[core][profiler]")
{
    Profiler::BeginFrame();

    {
        const ScopedTimer timer("SleepSection");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    REQUIRE(Profiler::GetResults().size() == 1);
    const float duration = Profiler::GetResults()[0].DurationMs;

    // Not asserting an exact value - sleep_for and steady_clock timing
    // both have OS-scheduler-dependent slop, and pinning this to an exact
    // millisecond count would make the test flaky on a loaded CI machine
    // for no real benefit. What matters structurally is that measured
    // time moved forward by roughly the requested amount, not that a
    // ScopedTimer somehow returned a duration close to zero (which is
    // the actual bug this test would catch: measuring at the wrong two
    // time points, or not measuring at all).
    REQUIRE(duration >= 4.0f);
}

TEST_CASE("ENGINE_PROFILE_SCOPE submits under the given name when its scope ends", "[core][profiler]")
{
    Profiler::BeginFrame();

    {
        ENGINE_PROFILE_SCOPE("First");
    }
    {
        ENGINE_PROFILE_SCOPE("Second");
    }

    const auto& results = Profiler::GetResults();
    REQUIRE(results.size() == 2);
    REQUIRE(results[0].Name == "First");
    REQUIRE(results[1].Name == "Second");
}

TEST_CASE("Two ENGINE_PROFILE_SCOPE uses in the same function do not collide on variable name", "[core][profiler]")
{
    // This is what the __LINE__-based unique naming in ENGINE_PROFILE_SCOPE
    // (see Profiler.h) exists to guarantee: two invocations on different
    // lines of the same function must not redeclare the same identifier,
    // which would simply fail to compile if the macro didn't disambiguate.
    Profiler::BeginFrame();

    ENGINE_PROFILE_SCOPE("Outer");
    {
        ENGINE_PROFILE_SCOPE("Inner");
    }

    REQUIRE(Profiler::GetResults().size() == 1); // only "Inner" has gone out of scope so far
}
