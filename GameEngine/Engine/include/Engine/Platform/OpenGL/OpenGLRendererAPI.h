#pragma once

#include "Engine/Renderer/RendererAPI.h"

namespace Engine
{
    class OpenGLRendererAPI final : public RendererAPI
    {
    public:
        void Init() override;
        void SetClearColor(const Math::Vec4& color) override;
        void Clear() override;
        void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, std::uint32_t indexCount) override;
    };

} // namespace Engine
