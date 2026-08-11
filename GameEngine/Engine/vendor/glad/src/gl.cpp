// Vendored GLAD (https://gen.glad.sh) - OpenGL 4.5 core, zero extensions.
// Generated once via `python -m glad --api gl:core=4.5 --reproducible c
// --header-only`, then committed like any other vendored dependency; GLAD
// is explicitly designed to be generated ahead of time rather than
// regenerated per build machine, unlike spdlog/GLFW which are fetched live
// via CMake FetchContent because they need no code generation step.
//
// This is the ONLY translation unit that may define GLAD_GL_IMPLEMENTATION:
// doing so pulls in the actual function-pointer definitions, and doing that
// in more than one translation unit would violate the One Definition Rule.
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
