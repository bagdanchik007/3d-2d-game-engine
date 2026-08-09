#include "Engine/Core/Time.h"
#include "Engine/Core/Assert.h"

#include <chrono>

namespace Engine
{
    namespace
    {
        // steady_clock, not system_clock: it cannot go backwards (e.g. on
        // NTP adjustment or a manual clock change), which system_clock can.
        // A frame delta that goes negative would corrupt any physics or
        // animation integration downstream.
        std::chrono::steady_clock::time_point s_StartTime;
    }

    bool Time::s_Initialized = false;

    void Time::Init()
    {
        s_StartTime = std::chrono::steady_clock::now();
        s_Initialized = true;
    }

    float Time::GetSeconds()
    {
        ENGINE_CORE_ASSERT(s_Initialized, "Time::Init() must be called before Time::GetSeconds()");

        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<float> elapsed = now - s_StartTime;
        return elapsed.count();
    }

} // namespace Engine
