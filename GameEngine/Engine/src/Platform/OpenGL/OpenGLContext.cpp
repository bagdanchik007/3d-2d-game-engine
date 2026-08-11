#include "Engine/Platform/OpenGL/OpenGLContext.h"

#include "Engine/Core/Assert.h"
#include "Engine/Core/Log.h"

#include <glad/gl.h>

#include <GLFW/glfw3.h>

namespace Engine
{
    OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) noexcept
        : m_WindowHandle(windowHandle)
    {
        ENGINE_CORE_ASSERT(windowHandle != nullptr, "OpenGLContext created with a null window handle");
    }

    void OpenGLContext::Init()
    {
        glfwMakeContextCurrent(m_WindowHandle);

        // gladLoadGL takes a function that resolves a GL function name to
        // its address; glfwGetProcAddress already has almost the right
        // signature (GLFWglproc vs GLADloadfunc are both `void(*)()`-family
        // typedefs), hence the reinterpret_cast rather than a hand-written
        // per-function shim.
        const int status = gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress));
        ENGINE_CORE_ASSERT(status != 0, "Failed to initialize GLAD");

        ENGINE_CORE_INFO("OpenGL Renderer: {} ({})", reinterpret_cast<const char*>(glGetString(GL_RENDERER)), reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    }

    void OpenGLContext::SwapBuffers()
    {
        glfwSwapBuffers(m_WindowHandle);
    }

} // namespace Engine
