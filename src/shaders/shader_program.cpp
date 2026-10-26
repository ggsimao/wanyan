#include "shaders/shader_program.h"

#include "lighting.h"

#include <cmath>

#include <format>
#include <iostream>
#include <stdexcept>
#include <utility>

ShaderProgram::ShaderProgram(std::string label, const VertexShader& vertex,
                             const FragmentShader& fragment)
    : _label(std::move(label)) {
    link({vertex.id(), fragment.id()});
}

ShaderProgram::ShaderProgram(std::string label, const VertexShader& vertex,
                             const GeometryShader& geometry, const FragmentShader& fragment)
    : _label(std::move(label)) {
    link({vertex.id(), geometry.id(), fragment.id()});
}

void ShaderProgram::link(std::initializer_list<GLuint> shaders) {
    _id = glCreateProgram();
    if (_id == 0) {
        throw std::runtime_error(std::format("Couldn't make shader program \"{}\"", _label));
    }

    for (GLuint shader : shaders) {
        glAttachShader(_id, shader);
    }
    glLinkProgram(_id);
    // Detached after linking so the shaders can be deleted as soon as their
    // C++ objects go out of scope; the program keeps the compiled result.
    for (GLuint shader : shaders) {
        glDetachShader(_id, shader);
    }

    GLint linked = GL_FALSE;
    glGetProgramiv(_id, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        // Called from a constructor, which won't run the destructor on throw.
        const std::string log = infoLog();
        release();
        throw std::runtime_error(std::format("Program \"{}\" link error: {}", _label, log));
    }

    // Names the program in GL's own debug messages and in graphics debuggers.
    // -1: the label is null-terminated.
    glObjectLabel(GL_PROGRAM, _id, -1, _label.c_str());
}

ShaderProgram ShaderProgram::fromFiles(std::string label, const std::filesystem::path& vertex,
                                       const std::filesystem::path& fragment) {
    return ShaderProgram(std::move(label), VertexShader::fromFile(vertex),
                         FragmentShader::fromFile(fragment));
}

ShaderProgram ShaderProgram::fromFiles(std::string label, const std::filesystem::path& vertex,
                                       const std::filesystem::path& geometry,
                                       const std::filesystem::path& fragment) {
    return ShaderProgram(std::move(label), VertexShader::fromFile(vertex),
                         GeometryShader::fromFile(geometry), FragmentShader::fromFile(fragment));
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept : _id(std::exchange(other._id, 0)) {}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
    if (this != &other) {
        release();
        _id = std::exchange(other._id, 0);
    }
    return *this;
}

ShaderProgram::~ShaderProgram() {
    release();
}

void ShaderProgram::release() noexcept {
    if (_id != 0) {
        glDeleteProgram(_id);
        _id = 0;
    }
}

GLuint ShaderProgram::id() const {
    return _id;
}

GLint ShaderProgram::uniformLocation(const std::string& name) const {
    const GLint location = glGetUniformLocation(_id, name.c_str());
    if (location == -1) {
        // Either the name isn't in the shader (a typo or a mismatch), or the
        // compiler removed it as unused. GL can't tell the two apart.
        std::cerr << "Warning: uniform \"" << name << "\" not found in program \"" << _label
                  << "\"\n";
    }
    return location;
}

void ShaderProgram::bind() const {
    glUseProgram(_id);
}

void ShaderProgram::clearBinding() {
    glUseProgram(0);
}

std::string ShaderProgram::infoLog() const {
    GLint length = 0;
    glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &length);
    std::string log(static_cast<std::size_t>(length), '\0');
    GLsizei written = 0;
    glGetProgramInfoLog(_id, length, &written, log.data());
    log.resize(static_cast<std::size_t>(written));
    return log;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderProgramBinding::setBool(const std::string& name, bool value) const {
    setInt(name, value ? 1 : 0);
}

void ShaderProgramBinding::setInt(const std::string& name, GLint value) const {
    assertActive();
    glUniform1i(_program.uniformLocation(name), value);
}

void ShaderProgramBinding::setFloat(const std::string& name, GLfloat value) const {
    assertActive();
    glUniform1f(_program.uniformLocation(name), value);
}

void ShaderProgramBinding::setVec3(const std::string& name, const glm::vec3& value) const {
    assertActive();
    glUniform3f(_program.uniformLocation(name), value.x, value.y, value.z);
}

void ShaderProgramBinding::setVec4(const std::string& name, const glm::vec4& value) const {
    assertActive();
    glUniform4f(_program.uniformLocation(name), value.x, value.y, value.z, value.w);
}

void ShaderProgramBinding::setMat3(const std::string& name, const glm::mat3& value) const {
    assertActive();
    glUniformMatrix3fv(_program.uniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

void ShaderProgramBinding::setMat4(const std::string& name, const glm::mat4& value) const {
    assertActive();
    glUniformMatrix4fv(_program.uniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

void ShaderProgramBinding::setMaterial(const std::string& name,
                                       const MaterialDrawBinding& material) const {
    const Material& m = material.material();
    const std::size_t diffuseCount = m.diffuseMaps().size();
    const std::size_t specularCount = m.specularMaps().size();

    for (std::size_t i = 0; i < diffuseCount; ++i) {
        setInt(std::format("{}.diffuseTextures[{}]", name, i), material.diffuseUnit(i));
    }
    for (std::size_t i = 0; i < specularCount; ++i) {
        setInt(std::format("{}.specularTextures[{}]", name, i), material.specularUnit(i));
    }
    setFloat(name + ".shininess", m.shininess());
    // Material guarantees at least one map of each kind, so these are >= 1.
    setInt(name + ".loadedDiffuse", static_cast<GLint>(diffuseCount));
    setInt(name + ".loadedSpecular", static_cast<GLint>(specularCount));
}

void ShaderProgramBinding::setDirectionalLight(const std::string& name,
                                               const DirectionalLight& light) const {
    setVec3(name + ".direction", light.direction);
    setLightColors(name, light);
}

void ShaderProgramBinding::setPointLight(const std::string& name, const PointLight& light) const {
    setVec3(name + ".position", light.position);
    setAttenuation(name, light.attenuation);
    setLightColors(name, light);
}

void ShaderProgramBinding::setSpotlight(const std::string& name, const Spotlight& light) const {
    setVec3(name + ".position", light.position);
    setVec3(name + ".direction", light.direction);
    setLightColors(name, light);
    // The shader compares cosines, which avoids an acos per fragment.
    setFloat(name + ".phiCos", std::cos(light.innerCutoff));
    setFloat(name + ".gammaCos", std::cos(light.outerCutoff));
}

void ShaderProgramBinding::setLightColors(const std::string& name,
                                          const LightColors& colors) const {
    setVec3(name + ".ambient", colors.effectiveAmbient());
    setVec3(name + ".diffuse", colors.effectiveDiffuse());
    setVec3(name + ".specular", colors.effectiveSpecular());
}

void ShaderProgramBinding::setAttenuation(const std::string& name,
                                          const Attenuation& attenuation) const {
    setFloat(name + ".constant", attenuation.constant);
    setFloat(name + ".linear", attenuation.linear);
    setFloat(name + ".quadratic", attenuation.quadratic);
}
