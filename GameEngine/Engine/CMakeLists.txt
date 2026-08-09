add_library(Engine STATIC
    src/Core/Log.cpp
    src/Core/Time.cpp
    src/Core/LayerStack.cpp
    src/Core/Application.cpp
    src/Platform/GLFW/GlfwWindow.cpp
    src/Platform/GLFW/GlfwInput.cpp
)

# PUBLIC: anything linking Engine (Sandbox, Tests) can #include "Engine/..."
# PRIVATE: Engine's own .cpp files use the same include root for symmetry.
target_include_directories(Engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# PUBLIC, not PRIVATE: Log.h's macros expand to spdlog::logger method calls
# at the *consumer's* call site (see the note in Log.h), so anything linking
# Engine needs spdlog's headers and link target transitively.
target_link_libraries(Engine
    PUBLIC
        spdlog::spdlog
    PRIVATE
        glfw
)

target_compile_definitions(Engine
    PUBLIC
        $<$<BOOL:${ENGINE_ENABLE_ASSERTS}>:ENGINE_ENABLE_ASSERTS>
)

engine_set_warnings(Engine)

set_target_properties(Engine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
