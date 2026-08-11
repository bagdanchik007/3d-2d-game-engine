#pragma once

#include "Engine/Renderer/Buffer.h"

#include <cstdint>

namespace Engine
{
    class OpenGLVertexBuffer final : public VertexBuffer
    {
    public:
        OpenGLVertexBuffer(const float* vertices, std::uint32_t size);
        explicit OpenGLVertexBuffer(std::uint32_t size);
        ~OpenGLVertexBuffer() override;

        OpenGLVertexBuffer(const OpenGLVertexBuffer&) = delete;
        OpenGLVertexBuffer& operator=(const OpenGLVertexBuffer&) = delete;

        void Bind() const override;
        void Unbind() const override;

        void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
        [[nodiscard]] const BufferLayout& GetLayout() const override { return m_Layout; }
        void SetData(const void* data, std::uint32_t size) override;

    private:
        std::uint32_t m_RendererID = 0;
        BufferLayout m_Layout;
    };

    class OpenGLIndexBuffer final : public IndexBuffer
    {
    public:
        OpenGLIndexBuffer(const std::uint32_t* indices, std::uint32_t count);
        ~OpenGLIndexBuffer() override;

        OpenGLIndexBuffer(const OpenGLIndexBuffer&) = delete;
        OpenGLIndexBuffer& operator=(const OpenGLIndexBuffer&) = delete;

        void Bind() const override;
        void Unbind() const override;

        [[nodiscard]] std::uint32_t GetCount() const override { return m_Count; }

    private:
        std::uint32_t m_RendererID = 0;
        std::uint32_t m_Count;
    };

} // namespace Engine
