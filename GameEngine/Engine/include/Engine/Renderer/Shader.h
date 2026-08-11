#pragma once

#include "Engine/Math/Math.h"

#include <cstdint>
#include <memory>
#include <string>

namespace Engine
{
    /// Compiled GLSL vertex+fragment program with a small set of typed
    /// uniform upload methods.
    ///
    /// Takes GLSL source directly (not a file path) deliberately: file
    /// loading is Milestone 9's ("Assets") job, not this class's. An
    /// AssetManager reading a .glsl file and handing the resulting string
    /// to Shader::Create is a trivial addition later precisely because this
    /// API doesn't currently assume where the source text came from.
    class Shader
    {
    public:
        virtual ~Shader() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual void SetInt(const std::string& name, int value) = 0;
        virtual void SetIntArray(const std::string& name, const int* values, uint32_t count) = 0;
        virtual void SetFloat3(const std::string& name, const Math::Vec3& value) = 0;
        virtual void SetFloat4(const std::string& name, const Math::Vec4& value) = 0;
        virtual void SetMat4(const std::string& name, const Math::Mat4& value) = 0;

        [[nodiscard]] static std::shared_ptr<Shader> Create(const std::string& vertexSource, const std::string& fragmentSource);
    };

} // namespace Engine
