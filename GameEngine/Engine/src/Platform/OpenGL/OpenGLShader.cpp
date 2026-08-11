#include "Engine/Platform/OpenGL/OpenGLShader.h"

#include "Engine/Core/Log.h"

#include <glad/gl.h>

#include <vector>

namespace Engine
{
    namespace
    {
        std::uint32_t CompileStage(GLenum stage, const std::string& source, const char* stageName)
        {
            const std::uint32_t shader = glCreateShader(stage);

            const char* src = source.c_str();
            glShaderSource(shader, 1, &src, nullptr);
            glCompileShader(shader);

            GLint isCompiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
            if (isCompiled == GL_FALSE)
            {
                GLint logLength = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

                std::vector<GLchar> infoLog(static_cast<std::size_t>(logLength));
                glGetShaderInfoLog(shader, logLength, nullptr, infoLog.data());

                ENGINE_CORE_ERROR("{} shader compilation failed:\n{}", stageName, infoLog.data());
                glDeleteShader(shader);
                return 0;
            }

            return shader;
        }
    }

    OpenGLShader::OpenGLShader(const std::string& vertexSource, const std::string& fragmentSource)
    {
        const std::uint32_t vertexShader = CompileStage(GL_VERTEX_SHADER, vertexSource, "Vertex");
        const std::uint32_t fragmentShader = CompileStage(GL_FRAGMENT_SHADER, fragmentSource, "Fragment");

        if (vertexShader == 0 || fragmentShader == 0)
        {
            // Partial failure cleanup: whichever stage DID compile must
            // still be deleted, or it leaks for the lifetime of the GL
            // context - a failed shader is still a real GL object until
            // explicitly destroyed.
            if (vertexShader != 0) { glDeleteShader(vertexShader); }
            if (fragmentShader != 0) { glDeleteShader(fragmentShader); }
            return; // m_RendererID stays 0; Bind() on a 0 program is a no-op in GL, not a crash.
        }

        m_RendererID = glCreateProgram();
        glAttachShader(m_RendererID, vertexShader);
        glAttachShader(m_RendererID, fragmentShader);
        glLinkProgram(m_RendererID);

        GLint isLinked = 0;
        glGetProgramiv(m_RendererID, GL_LINK_STATUS, &isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint logLength = 0;
            glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &logLength);

            std::vector<GLchar> infoLog(static_cast<std::size_t>(logLength));
            glGetProgramInfoLog(m_RendererID, logLength, nullptr, infoLog.data());
            ENGINE_CORE_ERROR("Shader program linking failed:\n{}", infoLog.data());

            glDeleteProgram(m_RendererID);
            m_RendererID = 0;
        }

        // Attached shader objects can be (and, per Khronos guidance,
        // should be) deleted right after a successful link: the program
        // keeps its own copy of everything it needs, and glDeleteShader on
        // an attached-but-not-yet-detached shader only marks it for
        // deletion once it is later detached/the program is destroyed.
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    OpenGLShader::~OpenGLShader()
    {
        glDeleteProgram(m_RendererID);
    }

    void OpenGLShader::Bind() const
    {
        glUseProgram(m_RendererID);
    }

    void OpenGLShader::Unbind() const
    {
        glUseProgram(0);
    }

    int OpenGLShader::GetUniformLocation(const std::string& name)
    {
        if (const auto it = m_UniformLocationCache.find(name); it != m_UniformLocationCache.end())
        {
            return it->second;
        }

        const int location = glGetUniformLocation(m_RendererID, name.c_str());
        m_UniformLocationCache[name] = location;
        return location;
    }

    void OpenGLShader::SetInt(const std::string& name, int value)
    {
        glUniform1i(GetUniformLocation(name), value);
    }

    void OpenGLShader::SetIntArray(const std::string& name, const int* values, uint32_t count)
    {
        glUniform1iv(GetUniformLocation(name), static_cast<GLsizei>(count), values);
    }

    void OpenGLShader::SetFloat3(const std::string& name, const Math::Vec3& value)
    {
        glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
    }

    void OpenGLShader::SetFloat4(const std::string& name, const Math::Vec4& value)
    {
        glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
    }

    void OpenGLShader::SetMat4(const std::string& name, const Math::Mat4& value)
    {
        // GL_FALSE (no transpose): Mat4 is already stored column-major with
        // GetData() returning a pointer in exactly the layout
        // glUniformMatrix4fv expects - the whole reason M4 chose that
        // storage order.
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, value.GetData());
    }

    std::shared_ptr<Shader> Shader::Create(const std::string& vertexSource, const std::string& fragmentSource)
    {
        return std::make_shared<OpenGLShader>(vertexSource, fragmentSource);
    }

} // namespace Engine
