# Vendored, pre-generated OpenGL loader - see src/gl.cpp for why this is
# checked in rather than fetched/generated at build time.
add_library(glad STATIC src/gl.cpp)

target_include_directories(glad PUBLIC include)

# GLAD's generated header trips a few of our stricter warnings (old-style
# casts, sign conversions inherent in the GL API's own function signatures);
# these are Khronos's function signatures and generator output, not code
# this project can fix, so silencing warnings here is scoped to this one
# vendored target only, never to project code.
if(NOT MSVC)
    target_compile_options(glad PRIVATE -w)
else()
    target_compile_options(glad PRIVATE /w)
endif()
