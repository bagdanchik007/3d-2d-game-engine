#include "Engine/Core/JobSystem.h"

namespace Engine
{
    JobSystem::JobSystem(std::size_t threadCount)
    {
        ENGINE_CORE_ASSERT(threadCount > 0, "JobSystem requires at least one worker thread");

        m_Threads.reserve(threadCount);
        for (std::size_t i = 0; i < threadCount; ++i)
        {
            m_Threads.emplace_back(&JobSystem::WorkerLoop, this);
        }
    }

    JobSystem::~JobSystem()
    {
        {
            const std::lock_guard<std::mutex> lock(m_QueueMutex);
            m_ShuttingDown = true;
        }
        // Every waiting worker must be woken, not just one: notify_one()
        // would leave the rest asleep on the condition variable forever,
        // since none of them would ever re-check m_ShuttingDown without
        // being woken - the classic reason a shutdown signal needs
        // notify_all() specifically, unlike Submit()'s notify_one() (there,
        // waking exactly one idle worker to claim exactly one new job is
        // the correct and sufficient behavior).
        m_Condition.notify_all();

        for (std::thread& thread : m_Threads)
        {
            thread.join();
        }
    }

    void JobSystem::WorkerLoop()
    {
        for (;;)
        {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);
                m_Condition.wait(lock, [this] { return m_ShuttingDown || !m_Jobs.empty(); });

                // Shutdown drains the queue before exiting, rather than
                // abandoning whatever was already submitted: a caller that
                // called Submit() and is waiting on the returned future
                // deserves that job to actually run (or at least be
                // attempted) during an orderly shutdown, not silently
                // dropped just because shutdown was also requested around
                // the same time.
                // The wait predicate above (m_ShuttingDown || !m_Jobs.empty())
                // is checked atomically under this same lock, so the only
                // way to observe an empty queue here is if m_ShuttingDown
                // is also true - there is no third case to handle.
                if (m_Jobs.empty())
                {
                    return;
                }

                job = std::move(m_Jobs.front());
                m_Jobs.pop();
            }

            job();
        }
    }

} // namespace Engine
