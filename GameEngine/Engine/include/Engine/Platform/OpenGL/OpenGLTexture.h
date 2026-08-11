#pragma once

#include "Engine/Renderer/Texture.h"

namespace Engine
{
    class OpenGLTexture2D final : public Texture2D
    {
    public:
        OpenGLTexture2D(uint32_t width, uint32_t height);
        explicit OpenGLTexture2D(const std::string& path);
        ~OpenGLTexture2D() override;

        OpenGLTexture2D(const OpenGLTexture2D&) = delete;
        OpenGLTexture2D& operator=(const OpenGLTexture2D&) = delete;

        [[nodiscard]] uint32_t GetWidth() const override { return m_Width; }
        [[nodiscard]] uint32_t GetHeight() const override { return m_Height; }

        void SetData(void* data, uint32_t size) override;
        void Bind(uint32_t slot) const override;

        [[nodiscard]] bool operator==(const Texture2D& other) const override
        {
            return m_RendererID == static_cast<const OpenGLTexture2D&>(other).m_RendererID;
        }

    private:
        std::string m_Path; // empty for procedurally-created textures - kept only for debugging/logging
        uint32_t m_Width;
        uint32_t m_Height;
        uint32_t m_RendererID = 0;
        uint32_t m_InternalFormat = 0;
        uint32_t m_DataFormat = 0;
    };

} // namespace Engine
