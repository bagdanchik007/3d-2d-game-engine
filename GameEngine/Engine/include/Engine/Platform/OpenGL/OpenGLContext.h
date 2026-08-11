#pragma once

#include "Engine/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Engine
{
    class OpenGLContext final : public GraphicsContext
    {
    public:
        explicit OpenGLContext(GLFWwindow* windowHandle) noexcept;

        void Init() override;
        void SwapBuffers() override;

    private:
        GLFWwindow* m_WindowHandle;
    };

} // namespace Engine
