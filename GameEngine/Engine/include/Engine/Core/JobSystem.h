#pragma once

#include "Engine/Core/Assert.h"

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

namespace Engine
{
    /// A general-purpose thread pool: N worker threads pulling from one
    /// shared job queue.
    ///
    /// Mutex + condition_variable, not a lock-free queue: this engine has
    /// no measured contention pattern that a lock-free structure would
    /// meaningfully improve, and a lock-free MPMC queue is a genuinely
    /// hard thing to get correct - reaching for one without a profiled
    /// need would be exactly the "invent complexity the requirements
    /// haven't asked for" this project avoids everywhere else (see
    /// PhysicsWorld's O(n^2) broad phase, M10, for the same reasoning
    /// applied to a different subsystem).
    ///
    /// This is a Core class, not a Renderer one: JobSystem itself knows
    /// nothing about assets, textures, or GL contexts. AssetManager (see
    /// Assets/AssetManager.h) is what layers "decode this file on a
    /// worker thread, upload it on the main thread" on top of a plain
    /// JobSystem - keeping JobSystem itself reusable for any CPU-only
    /// background work, not just asset loading.
    class JobSystem
    {
    public:
        explicit JobSystem(std::size_t threadCount = DefaultThreadCount());
        ~JobSystem();

        JobSystem(const JobSystem&) = delete;
        JobSystem& operator=(const JobSystem&) = delete;

        /// Submits a callable for execution on some worker thread and
        /// returns a std::future for its result. The callable runs
        /// exactly once, at some point after this call returns, on
        /// whichever worker thread picks it up next - never synchronously
        /// on the calling thread, even if a worker happens to be idle
        /// right now, since the job always goes through the queue.
        ///
        /// An exception thrown by the callable is captured by
        /// std::packaged_task and rethrown from future::get(), not lost -
        /// standard std::future behavior, not something this class adds,
        /// but worth calling out since JobSystem itself has no other
        /// error-reporting path for a job that fails.
        template <typename Func, typename... Args>
        auto Submit(Func&& func, Args&&... args) -> std::future<std::invoke_result_t<Func, Args...>>
        {
            using ReturnType = std::invoke_result_t<Func, Args...>;

            auto boundTask = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
            auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::move(boundTask));
            std::future<ReturnType> resultFuture = task->get_future();

            {
                const std::lock_guard<std::mutex> lock(m_QueueMutex);
                ENGINE_CORE_ASSERT(!m_ShuttingDown, "JobSystem::Submit called after shutdown was requested");
                m_Jobs.emplace([task]() { (*task)(); });
            }
            m_Condition.notify_one();

            return resultFuture;
        }

        [[nodiscard]] std::size_t GetThreadCount() const noexcept { return m_Threads.size(); }

        /// Never 0: hardware_concurrency() is documented to return 0 when
        /// it cannot determine the value, which would otherwise silently
        /// construct a JobSystem with zero worker threads - every
        /// submitted job would queue forever with nothing to ever run it,
        /// a hang with no error message pointing at the cause.
        [[nodiscard]] static std::size_t DefaultThreadCount() noexcept
        {
            const unsigned int detected = std::thread::hardware_concurrency();
            return detected > 0 ? detected : 1;
        }

    private:
        void WorkerLoop();

        std::vector<std::thread> m_Threads;
        std::queue<std::function<void()>> m_Jobs;
        std::mutex m_QueueMutex;
        std::condition_variable m_Condition;
        bool m_ShuttingDown = false;
    };

} // namespace Engine
