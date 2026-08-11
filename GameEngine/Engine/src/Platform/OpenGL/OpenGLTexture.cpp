#include "Engine/Platform/OpenGL/OpenGLTexture.h"

#include "Engine/Core/Assert.h"
#include "Engine/Core/Log.h"

#include <glad/gl.h>
#include <stb_image/stb_image.h>

namespace Engine
{
    OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
        : m_Width(width), m_Height(height), m_InternalFormat(GL_RGBA8), m_DataFormat(GL_RGBA)
    {
        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, static_cast<GLsizei>(m_Width), static_cast<GLsizei>(m_Height));

        // Nearest filtering, not linear: this is a pixel-art-oriented 2D
        // renderer (see Renderer2D.h) where blurring texture edges on
        // magnification is almost never the desired look. A future
        // material system could expose this as a per-texture setting; no
        // caller needs that distinction yet.
        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
        : m_Path(path), m_Width(0), m_Height(0)
    {
        int width = 0;
        int height = 0;
        int channels = 0;

        stbi_set_flip_vertically_on_load(1); // image files are top-down; OpenGL's texture coordinate origin is bottom-left
        stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        ENGINE_CORE_ASSERT(data != nullptr, "Failed to load texture");

        m_Width = static_cast<uint32_t>(width);
        m_Height = static_cast<uint32_t>(height);

        GLenum internalFormat = 0;
        GLenum dataFormat = 0;
        if (channels == 4)
        {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        }
        else if (channels == 3)
        {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        }
        ENGINE_CORE_ASSERT(internalFormat != 0, "Unsupported texture channel count");

        m_InternalFormat = internalFormat;
        m_DataFormat = dataFormat;

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, internalFormat, width, height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTextureSubImage2D(m_RendererID, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        glDeleteTextures(1, &m_RendererID);
    }

    void OpenGLTexture2D::SetData(void* data, uint32_t size)
    {
        const uint32_t bytesPerPixel = (m_DataFormat == GL_RGBA) ? 4 : 3;
        ENGINE_CORE_ASSERT(size == m_Width * m_Height * bytesPerPixel, "Data size does not match the entire texture");
        glTextureSubImage2D(m_RendererID, 0, 0, 0, static_cast<GLsizei>(m_Width), static_cast<GLsizei>(m_Height), m_DataFormat, GL_UNSIGNED_BYTE, data);
    }

    void OpenGLTexture2D::Bind(uint32_t slot) const
    {
        glBindTextureUnit(slot, m_RendererID);
    }

    std::shared_ptr<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
    {
        return std::make_shared<OpenGLTexture2D>(width, height);
    }

    std::shared_ptr<Texture2D> Texture2D::Create(const std::string& path)
    {
        return std::make_shared<OpenGLTexture2D>(path);
    }

} // namespace Engine
