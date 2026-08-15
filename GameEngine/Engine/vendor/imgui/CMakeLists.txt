# ImGui has no CMakeLists.txt of its own - see the FetchContent_Populate
# note in the root CMakeLists.txt for why this target is defined here by
# hand rather than via add_subdirectory on the fetched source.
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui SYSTEM PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# IMGUI_IMPL_OPENGL_LOADER_GLAD2 tells imgui_impl_opengl3.cpp to include
# <glad/gl.h> and trust it for OpenGL function pointers, rather than
# bundling and using its own default loader (gl3w). This is imgui's own
# purpose-built support for exactly the loader generation this project
# uses (glad2's gl.h + gladLoadGL - see Engine/vendor/glad), so there is no
# "two loaders fighting over the same context" conflict: only glad ever
# loads anything, and OpenGLContext::Init() (M6) already did so before any
# ImGuiLayer exists.
target_compile_definitions(imgui PUBLIC
    IMGUI_IMPL_OPENGL_LOADER_GLAD2=1
)

target_link_libraries(imgui PUBLIC glfw glad)

# Vendored third-party source - see the Engine/vendor/glad note on why
# warnings are silenced only for this one target, never for project code.
if(NOT MSVC)
    target_compile_options(imgui PRIVATE -w)
else()
    target_compile_options(imgui PRIVATE /w)
endif()
