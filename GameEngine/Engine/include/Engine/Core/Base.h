#pragma once

// ---------------------------------------------------------------------------
// Platform detection
// ---------------------------------------------------------------------------
// Kept minimal on purpose: we only detect what the engine actually branches
// on today. Adding detection for platforms we don't yet target would be
// speculative and untestable.
#if defined(_WIN32)
    #define ENGINE_PLATFORM_WINDOWS 1
#elif defined(__linux__)
    #define ENGINE_PLATFORM_LINUX 1
#elif defined(__APPLE__)
    #define ENGINE_PLATFORM_MACOS 1
#else
    #error "Engine: unsupported platform"
#endif

// ---------------------------------------------------------------------------
// Debug break
// ---------------------------------------------------------------------------
// Used by Assert.h to halt execution *at the call site* under a debugger,
// rather than unwinding into some generic abort handler that hides where
// the failure actually happened.
#if defined(ENGINE_PLATFORM_WINDOWS)
    #define ENGINE_DEBUGBREAK() __debugbreak()
#elif defined(__has_builtin)
    #if __has_builtin(__builtin_debugtrap)
        #define ENGINE_DEBUGBREAK() __builtin_debugtrap()
    #else
        #include <csignal>
        #define ENGINE_DEBUGBREAK() raise(SIGTRAP)
    #endif
#else
    #include <csignal>
    #define ENGINE_DEBUGBREAK() raise(SIGTRAP)
#endif

// ---------------------------------------------------------------------------
// Build configuration
// ---------------------------------------------------------------------------
#if defined(NDEBUG)
    #define ENGINE_BUILD_RELEASE 1
#else
    #define ENGINE_BUILD_DEBUG 1
#endif

// Bit helper, used later by input/event flag enums.
#define ENGINE_BIT(x) (1u << (x))
