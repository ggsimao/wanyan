#pragma once

#include "textures/draw_binding.h"
#include "scoped_binding.h"
#include "shaders/shader.h"

#include <GL/glew.h>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <filesystem>
#include <initializer_list>
#include <string>

struct Attenuation;
struct DirectionalLight;
struct LightColors;
struct PointLight;
struct Spotlight;

// A linked shader program. Linked (or throws, with the linker's log) on
// construction. Uniforms are set through a ShaderProgramBinding, because
// glUniform* acts on whichever program is currently in use.
class ShaderProgram {
public:
    ShaderProgram(std::string label, const VertexShader& vertex, const FragmentShader& fragment);
    ShaderProgram(std::string label, const VertexShader& vertex, const GeometryShader& geometry,
                  const FragmentShader& fragment);
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;
    ShaderProgram(ShaderProgram&& other) noexcept;
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;
    ~ShaderProgram();

    static ShaderProgram fromFiles(std::string label, const std::filesystem::path& vertex,
                                   const std::filesystem::path& fragment);
    static ShaderProgram fromFiles(std::string label, const std::filesystem::path& vertex,
                                   const std::filesystem::path& geometry, const std::filesystem::path& fragment);

    GLuint id() const;
    // -1 if the program has no active uniform with that name; setting -1 is a
    // no-op. Every miss prints a warning naming the uniform and the program.
    GLint uniformLocation(const std::string& name) const;

    void bind() const;
    static void clearBinding();

private:
    void link(std::initializer_list<GLuint> shaders);
    std::string infoLog() const;
    void release() noexcept;

    std::string _label;
    GLuint _id = 0;
};

// Puts a program in use for a scope and restores the previous one after.
// All uniform setters live here: they only work on the program in use.
class ShaderProgramBinding : ScopedBinding<ShaderProgram> {
public:
    explicit ShaderProgramBinding(const ShaderProgram& program)
        : ScopedBinding(program), _program(program) {}

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, GLint value) const;
    void setFloat(const std::string& name, GLfloat value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat3(const std::string& name, const glm::mat3& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;

    template <typename Texture>
    void setTexture(const std::string& name, const TextureDrawBinding<Texture>& texture) const {
        setInt(name, texture.unit());
    }
    void setMaterial(const std::string& name, const MaterialDrawBinding& material) const;

    // Shadow maps aren't set here: bind them with a TextureDrawBinding and
    // pass it to setTexture(name + ".shadow_map", ...).
    void setDirectionalLight(const std::string& name, const DirectionalLight& light) const;
    void setPointLight(const std::string& name, const PointLight& light) const;
    void setSpotlight(const std::string& name, const Spotlight& light) const;

private:
    void setLightColors(const std::string& name, const LightColors& colors) const;
    void setAttenuation(const std::string& name, const Attenuation& attenuation) const;
    const ShaderProgram& _program;
};
