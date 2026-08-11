#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace Engine
{
    /// GPU texture interface. Two creation paths:
    ///   - Create(path): loads image data from disk via stb_image.
    ///   - Create(width, height): allocates an empty texture the caller
    ///     fills via SetData - used for Renderer2D's 1x1 white texture and
    ///     for procedurally generated textures (Sandbox's checkerboard
    ///     demo), neither of which has a file to load from.
    /// Asset-level concerns (caching so the same file isn't loaded twice,
    /// reference-counted handles) are Milestone 9's job; this class is
    /// deliberately just "one GPU texture object", nothing more.
    class Texture2D
    {
    public:
        virtual ~Texture2D() = default;

        [[nodiscard]] virtual uint32_t GetWidth() const = 0;
        [[nodiscard]] virtual uint32_t GetHeight() const = 0;

        virtual void SetData(void* data, uint32_t size) = 0;

        virtual void Bind(uint32_t slot = 0) const = 0;

        [[nodiscard]] virtual bool operator==(const Texture2D& other) const = 0;

        [[nodiscard]] static std::shared_ptr<Texture2D> Create(uint32_t width, uint32_t height);
        [[nodiscard]] static std::shared_ptr<Texture2D> Create(const std::string& path);
    };

} // namespace Engine
