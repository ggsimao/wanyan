#pragma once

#include <GL/glew.h>

#include <filesystem>
#include <string>

enum class ShaderType : GLenum {
    VERTEX = GL_VERTEX_SHADER,
    GEOMETRY = GL_GEOMETRY_SHADER,
    FRAGMENT = GL_FRAGMENT_SHADER,
};

// One compiled shader stage. Compiled (or throws, with the compiler's log)
// on construction; only needed until it's linked into a ShaderProgram.
//
// The stage is part of the type, so ShaderProgram can require the right
// stages in the right order at compile time. The implementation lives in
// shader.cpp, explicitly instantiated for the three stages below.
template <ShaderType Type>
class Shader {
public:
    explicit Shader(const std::string& source);
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;
    ~Shader();

    static Shader fromFile(const std::filesystem::path& path);

    GLuint id() const;

private:
    std::string infoLog() const;
    void release() noexcept;

    GLuint _id = 0;
};

using VertexShader = Shader<ShaderType::VERTEX>;
using GeometryShader = Shader<ShaderType::GEOMETRY>;
using FragmentShader = Shader<ShaderType::FRAGMENT>;

extern template class Shader<ShaderType::VERTEX>;
extern template class Shader<ShaderType::GEOMETRY>;
extern template class Shader<ShaderType::FRAGMENT>;
