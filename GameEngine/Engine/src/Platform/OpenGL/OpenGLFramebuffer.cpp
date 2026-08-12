#include "Engine/Platform/OpenGL/OpenGLFramebuffer.h"

#include "Engine/Core/Assert.h"
#include "Engine/Core/Log.h"

#include <glad/gl.h>

namespace Engine
{
    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
        : m_Specification(spec)
    {
        Invalidate();
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        ReleaseAttachments();
        glDeleteFramebuffers(1, &m_RendererID);
    }

    void OpenGLFramebuffer::ReleaseAttachments() const noexcept
    {
        if (m_ColorAttachment != 0)
        {
            glDeleteTextures(1, &m_ColorAttachment);
        }
        if (m_DepthAttachment != 0)
        {
            glDeleteTextures(1, &m_DepthAttachment);
        }
    }

    void OpenGLFramebuffer::Invalidate()
    {
        if (m_RendererID != 0)
        {
            ReleaseAttachments();
            glDeleteFramebuffers(1, &m_RendererID);
        }

        glCreateFramebuffers(1, &m_RendererID);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachment);
        glTextureStorage2D(m_ColorAttachment, 1, GL_RGBA8,
            static_cast<GLsizei>(m_Specification.Width), static_cast<GLsizei>(m_Specification.Height));
        glTextureParameteri(m_ColorAttachment, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_ColorAttachment, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glNamedFramebufferTexture(m_RendererID, GL_COLOR_ATTACHMENT0, m_ColorAttachment, 0);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);
        glTextureStorage2D(m_DepthAttachment, 1, GL_DEPTH24_STENCIL8,
            static_cast<GLsizei>(m_Specification.Width), static_cast<GLsizei>(m_Specification.Height));
        glNamedFramebufferTexture(m_RendererID, GL_DEPTH_STENCIL_ATTACHMENT, m_DepthAttachment, 0);

        const GLenum status = glCheckNamedFramebufferStatus(m_RendererID, GL_FRAMEBUFFER);
        ENGINE_CORE_ASSERT(status == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete");
    }

    void OpenGLFramebuffer::Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        glViewport(0, 0, static_cast<GLsizei>(m_Specification.Width), static_cast<GLsizei>(m_Specification.Height));
    }

    void OpenGLFramebuffer::Unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
        {
            ENGINE_CORE_WARN("Attempted to resize framebuffer to {}x{} - ignored (0-sized attachments are invalid)", width, height);
            return;
        }

        m_Specification.Width = width;
        m_Specification.Height = height;
        Invalidate();
    }

    void OpenGLFramebuffer::BindColorAttachment(uint32_t slot) const
    {
        glBindTextureUnit(slot, m_ColorAttachment);
    }

    std::shared_ptr<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
    {
        return std::make_shared<OpenGLFramebuffer>(spec);
    }

} // namespace Engine
