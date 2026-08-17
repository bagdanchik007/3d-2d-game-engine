#pragma once

#include <spdlog/sinks/base_sink.h>

#include <deque>
#include <mutex>
#include <string>
#include <vector>

namespace Engine
{
    struct LogEntry
    {
        std::string Message;
        spdlog::level::level_enum Level;
    };

    /// A spdlog sink that captures formatted messages into a bounded ring
    /// buffer instead of writing them anywhere - the entire point being
    /// that ConsolePanel (M13) can display exactly what ENGINE_INFO/
    /// ENGINE_WARN/etc. already produce, without a second, parallel
    /// logging path that could drift out of sync with the real one.
    ///
    /// Inherits spdlog's own base_sink<std::mutex>, not a hand-rolled
    /// thread-safety scheme: base_sink already serializes calls to
    /// sink_it_/flush_ across threads, and duplicating that locking here
    /// would just be redoing work spdlog has already solved correctly.
    class ConsoleSink final : public spdlog::sinks::base_sink<std::mutex>
    {
    public:
        static constexpr std::size_t kMaxEntries = 1000;

        /// Returns a snapshot copy, not a reference to the live buffer:
        /// ConsolePanel reads this from the main thread during
        /// OnImGuiRender, but log calls (and therefore sink_it_) can
        /// happen from any thread at any time (M14's job system will make
        /// this a real, not hypothetical, concern) - handing back a copy
        /// under the same lock sink_it_ uses avoids ConsolePanel ever
        /// iterating a buffer that a concurrent log call is mutating.
        [[nodiscard]] std::vector<LogEntry> CopyEntries()
        {
            const std::lock_guard<std::mutex> lock(mutex_); // mutex_ is base_sink's own, protected member
            return std::vector<LogEntry>(m_Entries.begin(), m_Entries.end());
        }

        void Clear()
        {
            const std::lock_guard<std::mutex> lock(mutex_);
            m_Entries.clear();
        }

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override
        {
            spdlog::memory_buf_t formatted;
            base_sink<std::mutex>::formatter_->format(msg, formatted);

            m_Entries.push_back(LogEntry{fmt::to_string(formatted), msg.level});

            // Bounded ring buffer: an editor session left running for
            // hours must not let this grow without limit just because
            // nothing ever reads/clears it - pop the oldest entry, not
            // the newest, so the console always shows the most RECENT
            // history rather than freezing on whatever fit first.
            if (m_Entries.size() > kMaxEntries)
            {
                m_Entries.pop_front();
            }
        }

        void flush_() override {}

    private:
        std::deque<LogEntry> m_Entries;
    };

} // namespace Engine
