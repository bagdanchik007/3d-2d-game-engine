#pragma once

#include "Engine/Renderer/Shader.h"

#include <cstdint>
#include <unordered_map>

namespace Engine
{
    class OpenGLShader final : public Shader
    {
    public:
        OpenGLShader(const std::string& vertexSource, const std::string& fragmentSource);
        ~OpenGLShader() override;

        OpenGLShader(const OpenGLShader&) = delete;
        OpenGLShader& operator=(const OpenGLShader&) = delete;

        void Bind() const override;
        void Unbind() const override;

        void SetInt(const std::string& name, int value) override;
        void SetIntArray(const std::string& name, const int* values, uint32_t count) override;
        void SetFloat3(const std::string& name, const Math::Vec3& value) override;
        void SetFloat4(const std::string& name, const Math::Vec4& value) override;
        void SetMat4(const std::string& name, const Math::Mat4& value) override;

    private:
        /// Uniform locations are looked up by name via a driver call
        /// (glGetUniformLocation) that is not free; caching them per shader
        /// avoids repeating that lookup every time the same uniform is set,
        /// which for something like a per-object model matrix can be once
        /// per draw call - a real, not hypothetical, per-frame cost.
        [[nodiscard]] int GetUniformLocation(const std::string& name);

        std::uint32_t m_RendererID = 0;
        std::unordered_map<std::string, int> m_UniformLocationCache;
    };

} // namespace Engine
