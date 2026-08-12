#pragma once

#include <cstdint>
#include <memory>

namespace Engine
{
    struct FramebufferSpecification
    {
        uint32_t Width = 0;
        uint32_t Height = 0;
    };

    /// An off-screen render target: color + depth attachments, resizable.
    ///
    /// The color attachment is exposed via BindColorAttachment(slot), not
    /// as a Texture2D: forcing a framebuffer's attachment through the
    /// Texture2D interface (which supports SetData(), file loading, and is
    /// meant to represent an owned, standalone texture) would be shoehorning
    /// two different lifetimes and write-access models into one
    /// abstraction just because both happen to end up as a GL texture name
    /// under the hood. A caller that wants to display this attachment
    /// binds the slot directly - see Sandbox's FramebufferBlit for the
    /// dedicated (non-Renderer2D) path that does exactly that.
    class Framebuffer
    {
    public:
        virtual ~Framebuffer() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        /// Recreates both attachments at the new size. Not a resizable
        /// texture in place - GL texture storage is immutable once
        /// allocated (see OpenGLTexture2D's glTextureStorage2D), so a
        /// resize is unavoidably "destroy and recreate", not "reallocate".
        virtual void Resize(uint32_t width, uint32_t height) = 0;

        virtual void BindColorAttachment(uint32_t slot = 0) const = 0;

        [[nodiscard]] virtual const FramebufferSpecification& GetSpecification() const = 0;

        [[nodiscard]] static std::shared_ptr<Framebuffer> Create(const FramebufferSpecification& spec);
    };

} // namespace Engine
