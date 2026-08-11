#pragma once

#include "Engine/Renderer/RendererAPI.h"

#include <memory>

namespace Engine
{
    /// Static facade wrapping a single RendererAPI instance, the same
    /// "thin static wrapper over one polymorphic instance" shape as
    /// Engine::Log wrapping spdlog::logger. Call sites write
    /// `RenderCommand::Clear()` instead of threading a RendererAPI&
    /// through every function that draws anything - there is exactly one
    /// active RendererAPI for the process's lifetime, so a singleton-style
    /// facade costs nothing here that passing it explicitly would have
    /// bought back.
    class RenderCommand
    {
    public:
        RenderCommand() = delete;

        static void Init();
        static void SetClearColor(const Math::Vec4& color);
        static void Clear();
        static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, std::uint32_t indexCount = 0);

    private:
        static std::unique_ptr<RendererAPI> s_RendererAPI;
    };

} // namespace Engine
