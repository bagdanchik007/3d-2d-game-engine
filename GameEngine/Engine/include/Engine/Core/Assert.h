#pragma once

#include "Engine/Core/Base.h"
#include "Engine/Core/Log.h"

// ---------------------------------------------------------------------------
// ENGINE_ASSERT / ENGINE_CORE_ASSERT
// ---------------------------------------------------------------------------
// Deliberately separate from Log.h even though it depends on it: logging is
// "always on" infrastructure, assertions are a *policy* (compiled out in
// Release via ENGINE_ENABLE_ASSERTS) layered on top of it. Merging the two
// headers would force every logging call site to also carry assertion
// semantics it doesn't need.
//
// __VA_OPT__ (C++20) lets the optional message argument disappear cleanly
// when not supplied, without relying on the non-standard `, ##__VA_ARGS__`
// GNU extension.
#if defined(ENGINE_ENABLE_ASSERTS)

    // NOTE on a bug caught during verification, not a mistake left silent:
    // An earlier version embedded the caller's message format string as an
    // *argument value* inside an outer "Assertion failed: {} - {}" template.
    // That doesn't work — fmt only runs one formatting pass, so the caller's
    // own "{}" placeholders were never substituted and printed literally.
    // The fix is two independent log calls: the condition text always gets
    // logged as its own complete statement, and the optional caller message
    // (with its own placeholders) is formatted as its own complete statement.
    #define ENGINE_INTERNAL_ASSERT_IMPL(logger_call, check, ...)               \
        do                                                                      \
        {                                                                        \
            if (!(check))                                                        \
            {                                                                     \
                logger_call("Assertion failed: {}", #check);                       \
                __VA_OPT__(logger_call(__VA_ARGS__);)                               \
                ENGINE_DEBUGBREAK();                                                 \
            }                                                                         \
        } while (false)

    #define ENGINE_ASSERT(check, ...)      ENGINE_INTERNAL_ASSERT_IMPL(ENGINE_ERROR, check, __VA_ARGS__)
    #define ENGINE_CORE_ASSERT(check, ...) ENGINE_INTERNAL_ASSERT_IMPL(ENGINE_CORE_ERROR, check, __VA_ARGS__)

#else

    #define ENGINE_ASSERT(check, ...)
    #define ENGINE_CORE_ASSERT(check, ...)

#endif
