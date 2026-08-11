#pragma once

#include "Engine/Math/Vec4.h"
#include "Engine/Renderer/VertexArray.h"

#include <memory>

namespace Engine
{
    /// The minimal set of operations a graphics backend must implement to
    /// back the renderer. Deliberately small for M6 - no BeginScene/camera/
    /// submission-queue concepts here, since those are Milestone 7/8 scope
    /// once there's an actual Renderer built on top of this.
    class RendererAPI
    {
    public:
        enum class API
        {
            None = 0,
            OpenGL,
        };

        virtual ~RendererAPI() = default;

        virtual void Init() = 0;
        virtual void SetClearColor(const Math::Vec4& color) = 0;
        virtual void Clear() = 0;
        virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, std::uint32_t indexCount = 0) = 0;

        /// Compile-time-fixed for now: OpenGL is the only backend, so
        /// there is nothing to switch at runtime yet. This exists as the
        /// seam a second backend would plug into (mirroring Window::Create
        /// in M3), not as a currently-exercised switch.
        [[nodiscard]] static API GetAPI() noexcept { return s_API; }

        [[nodiscard]] static std::unique_ptr<RendererAPI> Create();

    private:
        static constexpr API s_API = API::OpenGL;
    };

} // namespace Engine
