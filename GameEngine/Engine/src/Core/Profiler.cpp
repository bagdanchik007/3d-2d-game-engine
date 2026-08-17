#include "Engine/Core/Profiler.h"

namespace Engine
{
    std::vector<ProfileResult> Profiler::s_Results;
    float Profiler::s_LastFrameTimeMs = 0.0f;

} // namespace Engine
