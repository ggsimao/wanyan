#include "shaders/shader.h"

#include "utils.h"

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace {
    constexpr const char* stageName(ShaderType type) {
        switch (type) {
            case ShaderType::VERTEX:   return "Vertex";
            case ShaderType::GEOMETRY: return "Geometry";
            case ShaderType::FRAGMENT: return "Fragment";
        }
        return "Unknown";  // unreachable; keeps -Wreturn-type quiet
    }
}  // namespace

template <ShaderType Type>
Shader<Type>::Shader(const std::string& source) {
    _id = glCreateShader(static_cast<GLenum>(Type));
    if (_id == 0) {
        throw std::runtime_error("Couldn't make a shader");
    }

    const char* text = source.c_str();
    const auto length = static_cast<GLint>(source.size());
    glShaderSource(_id, 1, &text, &length);
    glCompileShader(_id);

    GLint compiled = GL_FALSE;
    glGetShaderiv(_id, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        // A throwing constructor never runs the destructor: free it here.
        const std::string log = infoLog();
        release();
        throw std::runtime_error(std::string(stageName(Type)) + " compile error: " + log);
    }
}

template <ShaderType Type>
Shader<Type> Shader<Type>::fromFile(const std::filesystem::path& path) {
    try {
        return Shader(readFile(path));
    } catch (const std::runtime_error& error) {
        throw std::runtime_error(path.string() + ": " + error.what());
    }
}

template <ShaderType Type>
Shader<Type>::Shader(Shader&& other) noexcept : _id(std::exchange(other._id, 0)) {}

template <ShaderType Type>
Shader<Type>& Shader<Type>::operator=(Shader&& other) noexcept {
    if (this != &other) {
        release();
        _id = std::exchange(other._id, 0);
    }
    return *this;
}

template <ShaderType Type>
Shader<Type>::~Shader() {
    release();
}

template <ShaderType Type>
void Shader<Type>::release() noexcept {
    if (_id != 0) {
        glDeleteShader(_id);
        _id = 0;
    }
}

template <ShaderType Type>
GLuint Shader<Type>::id() const {
    return _id;
}

template <ShaderType Type>
std::string Shader<Type>::infoLog() const {
    GLint length = 0;
    glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &length);
    std::string log(static_cast<std::size_t>(length), '\0');
    GLsizei written = 0;
    glGetShaderInfoLog(_id, length, &written, log.data());
    log.resize(static_cast<std::size_t>(written));
    return log;
}

// The only stages that exist; each one's code is generated here, once.
template class Shader<ShaderType::VERTEX>;
template class Shader<ShaderType::GEOMETRY>;
template class Shader<ShaderType::FRAGMENT>;
