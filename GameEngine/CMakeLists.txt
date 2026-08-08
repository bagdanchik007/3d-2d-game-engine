cmake_minimum_required(VERSION 3.24)

project(GameEngine
    VERSION 0.1.0
    DESCRIPTION "Portfolio 2D/3D Game Engine"
    LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Emit compile_commands.json for tooling (clangd, clang-tidy, IDEs).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Keep all binaries in one place regardless of subdirectory nesting.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ---------------------------------------------------------------------------
# Project-wide options
# ---------------------------------------------------------------------------
option(ENGINE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(ENGINE_ENABLE_ASSERTS "Compile ENGINE_ASSERT checks into the binary" ON)

# Default to a sane build type for single-config generators (Make/Ninja).
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

include(cmake/CompilerWarnings.cmake)

# ---------------------------------------------------------------------------
# Third-party dependencies (reproducible via FetchContent, no system installs)
# ---------------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.14.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(spdlog)

# ---------------------------------------------------------------------------
# Subprojects
# ---------------------------------------------------------------------------
add_subdirectory(Engine)
add_subdirectory(Sandbox)
