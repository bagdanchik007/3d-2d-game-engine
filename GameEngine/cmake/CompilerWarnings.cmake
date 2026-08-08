# Applies a strict, consistent warning set to a given target.
# Kept in one place so every target (Engine, Sandbox, Tests) is held to the
# same bar instead of copy-pasting flag lists into each CMakeLists.txt.
function(engine_set_warnings target_name)
    set(MSVC_WARNINGS
        /W4
        /permissive-
    )

    set(CLANG_GCC_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
    )

    if(ENGINE_WARNINGS_AS_ERRORS)
        list(APPEND MSVC_WARNINGS /WX)
        list(APPEND CLANG_GCC_WARNINGS -Werror)
    endif()

    if(MSVC)
        target_compile_options(${target_name} PRIVATE ${MSVC_WARNINGS})
    else()
        target_compile_options(${target_name} PRIVATE ${CLANG_GCC_WARNINGS})
    endif()
endfunction()
