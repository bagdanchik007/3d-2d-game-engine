#pragma once

namespace Engine
{
    /// Owns the graphics API context tied to a native window: OpenGL
    /// function loading, and presenting a completed frame.
    ///
    /// Split out of GlfwWindow (which handled this directly and adequately
    /// through M3-M5, when nothing issued a real GL call) for the same
    /// reason Window itself is an interface over GLFW: "which windowing
    /// system" and "which graphics API" are independent axes. A future
    /// Vulkan backend would still use GLFW for the window but needs an
    /// entirely different context object (no GL function loading, a
    /// VkSurfaceKHR instead) - that swap should not touch GlfwWindow at all.
    class GraphicsContext
    {
    public:
        virtual ~GraphicsContext() = default;

        /// Makes this context current and loads graphics API function
        /// pointers. Separate from construction so GlfwWindow can create
        /// the GLFW window first (required before a context can be made
        /// current on it) and then call Init() once the native handle
        /// exists - construction-order coupling made explicit as two steps
        /// rather than hidden inside one constructor.
        virtual void Init() = 0;

        virtual void SwapBuffers() = 0;
    };

} // namespace Engine
