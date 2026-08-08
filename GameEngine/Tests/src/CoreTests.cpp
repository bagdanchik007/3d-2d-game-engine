#include "Engine/Core/Assert.h"
#include "Engine/Core/Log.h"

#include <catch2/catch_test_macros.hpp>

// Log::Init() must run exactly once before any test that touches the
// loggers. Catch2 doesn't guarantee test order, so this is done via a
// file-scope helper invoked from each test that needs it rather than a
// global fixture spanning the whole binary — keeps the dependency explicit
// at each call site instead of hidden in test-runner setup.
namespace
{
    void EnsureLogInitialized()
    {
        static const bool initialized = []
        {
            Engine::Log::Init();
            return true;
        }();
        (void)initialized;
    }
}

TEST_CASE("Log::Init creates non-null Core and Client loggers", "[core][log]")
{
    EnsureLogInitialized();

    REQUIRE(Engine::Log::GetCoreLogger() != nullptr);
    REQUIRE(Engine::Log::GetClientLogger() != nullptr);
}

TEST_CASE("Core and Client loggers are distinct instances", "[core][log]")
{
    EnsureLogInitialized();

    // They must not be the same logger: mixing engine and app messages
    // under one tag would defeat the entire point of separating them
    // (see the design rationale in Log.h).
    REQUIRE(Engine::Log::GetCoreLogger() != Engine::Log::GetClientLogger());
}

TEST_CASE("ENGINE_ASSERT does not abort the program when the condition holds", "[core][assert]")
{
    EnsureLogInitialized();

    int value = 10;
    // A passing assertion must be a true no-op: if this macro were broken
    // and evaluated its message arguments unconditionally with side
    // effects, or debug-broke on success, this test would never complete.
    ENGINE_ASSERT(value == 10, "value should be 10, was {}", value);

    SUCCEED("Reached this point without the debug break firing");
}
