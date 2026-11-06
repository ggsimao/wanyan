#include "shaders/shader_program.h"

#include "lighting.h"
#include "shaders/uniform_name.h"

#include <cmath>

#include <format>
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

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept : _label(std::move(other._label)),
                                                               _id(std::exchange(other._id, 0)),
                                                               _cache(std::move(other._cache)) {}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
    if (this != &other) {
        release();
        _label = std::move(other._label);
        _id = std::exchange(other._id, 0);
        _cache = std::move(other._cache);
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

GLint ShaderProgram::uniformLocation(std::string_view name) const {
    return _cache.uniformLocation(name, _label, _id);
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

void ShaderProgramBinding::setBool(std::string_view name, bool value) const {
    setInt(name, value ? 1 : 0);
}

void ShaderProgramBinding::setInt(std::string_view name, GLint value) const {
    assertActive();
    glUniform1i(_program.uniformLocation(name), value);
}

void ShaderProgramBinding::setFloat(std::string_view name, GLfloat value) const {
    assertActive();
    glUniform1f(_program.uniformLocation(name), value);
}

void ShaderProgramBinding::setVec3(std::string_view name, const glm::vec3& value) const {
    assertActive();
    glUniform3f(_program.uniformLocation(name), value.x, value.y, value.z);
}

void ShaderProgramBinding::setVec4(std::string_view name, const glm::vec4& value) const {
    assertActive();
    glUniform4f(_program.uniformLocation(name), value.x, value.y, value.z, value.w);
}

void ShaderProgramBinding::setMat3(std::string_view name, const glm::mat3& value) const {
    assertActive();
    glUniformMatrix3fv(_program.uniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

void ShaderProgramBinding::setMat4(std::string_view name, const glm::mat4& value) const {
    assertActive();
    glUniformMatrix4fv(_program.uniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

void ShaderProgramBinding::setMaterial(std::string_view name,
                                       const MaterialDrawBinding& material) const {
    const Material& m = material.material();
    const std::size_t diffuseCount = m.diffuseMaps().size();
    const std::size_t specularCount = m.specularMaps().size();

    for (std::size_t i = 0; i < diffuseCount; ++i) {
        UniformName diffuse("{}.diffuseTextures[{}]", name, i);
        setInt(diffuse.view(), material.diffuseUnit(i));
    }
    for (std::size_t i = 0; i < specularCount; ++i) {
        UniformName specular("{}.specularTextures[{}]", name, i);
        setInt(specular.view(), material.specularUnit(i));
    }
    UniformName shininess("{}.shininess", name);
    setFloat(shininess.view(), m.shininess());
    // Material guarantees at least one map of each kind, so these are >= 1.
    UniformName loadedDiffuse("{}.loadedDiffuse", name);
    UniformName loadedSpecular("{}.loadedSpecular", name);
    setInt(loadedDiffuse.view(), static_cast<GLint>(diffuseCount));
    setInt(loadedSpecular.view(), static_cast<GLint>(specularCount));
}

void ShaderProgramBinding::setDirectionalLight(std::string_view name,
                                               const DirectionalLight& light) const {
    UniformName direction("{}.direction", name);
    setVec3(direction.view(), light.direction);
    setLightRadiance(name, light);
}

void ShaderProgramBinding::setPointLight(std::string_view name, const PointLight& light) const {
    UniformName position("{}.position", name);
    setVec3(position.view(), light.position);
    setAttenuation(name, light.attenuation);
    setLightRadiance(name, light);
}

void ShaderProgramBinding::setSpotlight(std::string_view name, const Spotlight& light) const {
    UniformName position("{}.position", name);
    UniformName direction("{}.direction", name);
    setVec3(position.view(), light.position);
    setVec3(direction.view(), light.direction);
    setLightRadiance(name, light);
    setAttenuation(name, light.attenuation);
    // The shader compares cosines, which avoids an acos per fragment.
    UniformName phiCos("{}.phiCos", name);
    UniformName gammaCos("{}.gammaCos", name);
    setFloat(phiCos.view(), std::cos(light.innerCutoff));
    setFloat(gammaCos.view(), std::cos(light.outerCutoff));
}

void ShaderProgramBinding::setLightRadiance(std::string_view name,
                                          const Light& color) const {
    UniformName radiance("{}.radiance", name);
    setVec3(radiance.view(), color.effectiveRadiance());
}

void ShaderProgramBinding::setAttenuation(std::string_view name,
                                          const Attenuation& attenuation) const {
    UniformName constant("{}.attenuation.constant", name);
    UniformName linear("{}.attenuation.linear", name);
    UniformName quadratic("{}.attenuation.quadratic", name);
    setFloat(constant.view(), attenuation.constant);
    setFloat(linear.view(), attenuation.linear);
    setFloat(quadratic.view(), attenuation.quadratic);
}
