#include "Engine/Core/ConsoleSink.h"

#include <catch2/catch_test_macros.hpp>

#include <spdlog/spdlog.h>

using namespace Engine;

namespace
{
    /// A fresh logger wired to its own ConsoleSink instance, independent
    /// of Engine::Log's process-wide loggers - so these tests can log
    /// freely (including enough messages to overflow the ring buffer)
    /// without disturbing or depending on Log::Init()'s global state,
    /// which other test files also touch.
    struct SinkFixture
    {
        std::shared_ptr<ConsoleSink> Sink = std::make_shared<ConsoleSink>();
        std::shared_ptr<spdlog::logger> Logger = std::make_shared<spdlog::logger>("test", Sink);
    };
}

TEST_CASE("Logging through a logger with a ConsoleSink attached captures the message", "[core][consolesink]")
{
    SinkFixture fixture;
    fixture.Logger->info("hello console");

    const auto entries = fixture.Sink->CopyEntries();
    REQUIRE(entries.size() == 1);
    REQUIRE(entries[0].Level == spdlog::level::info);
    REQUIRE(entries[0].Message.find("hello console") != std::string::npos);
}

TEST_CASE("Multiple log calls are captured in order", "[core][consolesink]")
{
    SinkFixture fixture;
    fixture.Logger->info("first");
    fixture.Logger->warn("second");
    fixture.Logger->error("third");

    const auto entries = fixture.Sink->CopyEntries();
    REQUIRE(entries.size() == 3);
    REQUIRE(entries[0].Message.find("first") != std::string::npos);
    REQUIRE(entries[1].Level == spdlog::level::warn);
    REQUIRE(entries[2].Level == spdlog::level::err);
}

TEST_CASE("Clear empties the captured entries", "[core][consolesink]")
{
    SinkFixture fixture;
    fixture.Logger->info("will be cleared");
    REQUIRE(fixture.Sink->CopyEntries().size() == 1);

    fixture.Sink->Clear();
    REQUIRE(fixture.Sink->CopyEntries().empty());
}

TEST_CASE("The ring buffer never grows past kMaxEntries, dropping the OLDEST entries first", "[core][consolesink]")
{
    SinkFixture fixture;

    const std::size_t countToLog = ConsoleSink::kMaxEntries + 10;
    for (std::size_t i = 0; i < countToLog; ++i)
    {
        fixture.Logger->info("message {}", i);
    }

    const auto entries = fixture.Sink->CopyEntries();
    REQUIRE(entries.size() == ConsoleSink::kMaxEntries);

    // The oldest 10 messages (0..9) must have been evicted; the buffer
    // should start at message 10 and end at message (countToLog - 1) -
    // checking this, not just the size, is what actually verifies
    // "oldest dropped first" rather than some other eviction order that
    // happened to leave the right COUNT of entries.
    REQUIRE(entries.front().Message.find("message 10") != std::string::npos);
    REQUIRE(entries.back().Message.find("message " + std::to_string(countToLog - 1)) != std::string::npos);
}
