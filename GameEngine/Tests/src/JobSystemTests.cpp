#include "Engine/Core/JobSystem.h"

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <vector>

using namespace Engine;

TEST_CASE("DefaultThreadCount is never zero", "[core][jobsystem]")
{
    // hardware_concurrency() is documented to return 0 when it cannot
    // determine the value - this is the one behavior of DefaultThreadCount
    // that's actually testable without mocking the standard library:
    // whatever the real value is, it must never be 0, or a
    // default-constructed JobSystem would have no worker threads at all.
    REQUIRE(JobSystem::DefaultThreadCount() > 0);
}

TEST_CASE("A submitted job runs and its future resolves to the correct result", "[core][jobsystem]")
{
    JobSystem jobSystem(2);

    std::future<int> result = jobSystem.Submit([]() { return 21 * 2; });

    REQUIRE(result.get() == 42);
}

TEST_CASE("Submit forwards arguments to the callable", "[core][jobsystem]")
{
    JobSystem jobSystem(2);

    std::future<int> result = jobSystem.Submit([](int a, int b) { return a + b; }, 10, 32);

    REQUIRE(result.get() == 42);
}

TEST_CASE("A job that throws propagates the exception through future::get", "[core][jobsystem]")
{
    JobSystem jobSystem(2);

    std::future<int> result = jobSystem.Submit([]() -> int { throw std::runtime_error("job failed"); });

    REQUIRE_THROWS_AS(result.get(), std::runtime_error);
}

TEST_CASE("Many submitted jobs all complete exactly once, with correct individual results", "[core][jobsystem]")
{
    JobSystem jobSystem(4);

    constexpr int kJobCount = 200;
    std::vector<std::future<int>> futures;
    futures.reserve(kJobCount);

    for (int i = 0; i < kJobCount; ++i)
    {
        futures.push_back(jobSystem.Submit([i]() { return i * i; }));
    }

    for (int i = 0; i < kJobCount; ++i)
    {
        REQUIRE(futures[static_cast<std::size_t>(i)].get() == i * i);
    }
}

TEST_CASE("Concurrently submitted jobs that each increment a shared atomic counter produce the exact expected total", "[core][jobsystem]")
{
    // This is the test that actually exercises real cross-thread
    // correctness, not just "did the API return the right value":
    // hundreds of jobs across multiple worker threads incrementing one
    // shared counter would reveal a genuine data race (a missing memory
    // barrier, a broken queue) as a wrong final count - a plain
    // int++ (not atomic) here would very likely fail this test under
    // real concurrency, which is exactly why std::atomic is used instead,
    // not a redundant precaution.
    JobSystem jobSystem(8);

    std::atomic<int> counter{0};
    constexpr int kJobCount = 1000;

    std::vector<std::future<void>> futures;
    futures.reserve(kJobCount);
    for (int i = 0; i < kJobCount; ++i)
    {
        futures.push_back(jobSystem.Submit([&counter]() { counter.fetch_add(1, std::memory_order_relaxed); }));
    }

    for (auto& future : futures)
    {
        future.get();
    }

    REQUIRE(counter.load() == kJobCount);
}

TEST_CASE("JobSystem destruction waits for already-submitted jobs to complete", "[core][jobsystem]")
{
    std::atomic<int> completedCount{0};

    {
        JobSystem jobSystem(4);
        for (int i = 0; i < 50; ++i)
        {
            jobSystem.Submit([&completedCount]()
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                completedCount.fetch_add(1, std::memory_order_relaxed);
            });
        }
        // jobSystem destructs here - its destructor must block until every
        // worker thread has drained the queue and joined, not tear down
        // mid-job.
    }

    REQUIRE(completedCount.load() == 50);
}

TEST_CASE("GetThreadCount reflects the count passed to the constructor", "[core][jobsystem]")
{
    const JobSystem jobSystem(3);
    REQUIRE(jobSystem.GetThreadCount() == 3);
}
