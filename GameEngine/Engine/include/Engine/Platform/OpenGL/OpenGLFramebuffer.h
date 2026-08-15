#pragma once

#include "Engine/Renderer/Framebuffer.h"

namespace Engine
{
    class OpenGLFramebuffer final : public Framebuffer
    {
    public:
        explicit OpenGLFramebuffer(const FramebufferSpecification& spec);
        ~OpenGLFramebuffer() override;

        OpenGLFramebuffer(const OpenGLFramebuffer&) = delete;
        OpenGLFramebuffer& operator=(const OpenGLFramebuffer&) = delete;

        void Bind() const override;
        void Unbind() const override;
        void Resize(uint32_t width, uint32_t height) override;
        void BindColorAttachment(uint32_t slot) const override;
        [[nodiscard]] uint32_t GetColorAttachmentRendererID() const override { return m_ColorAttachment; }

        [[nodiscard]] const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

    private:
        void Invalidate();
        void ReleaseAttachments() const noexcept;

        FramebufferSpecification m_Specification;
        uint32_t m_RendererID = 0;
        uint32_t m_ColorAttachment = 0;
        uint32_t m_DepthAttachment = 0;
    };

} // namespace Engine
