#include "shaders/point_shadow_shader.h"

#include <utility>

namespace {
    // One layer per cube face. Must match FACES in
    // shaders/pointshadow_layered_vert_shader.vs.
    constexpr GLsizei CUBE_FACES = 6;
}  // namespace

PointShadowShader
PointShadowShader::layered(std::string label,
                           const std::filesystem::path& vertex,
                           const std::filesystem::path& fragment) {
    return PointShadowShader(
        ShaderProgram::fromFiles(std::move(label), vertex, fragment),
        CUBE_FACES);
}

PointShadowShader
PointShadowShader::withGeometryShader(std::string label,
                                      const std::filesystem::path& vertex,
                                      const std::filesystem::path& geometry,
                                      const std::filesystem::path& fragment) {
    return PointShadowShader(
        ShaderProgram::fromFiles(std::move(label), vertex, geometry, fragment),
        1);
}

PointShadowShader::PointShadowShader(ShaderProgram program, GLsizei layers)
    : _program(std::move(program)), _layers(layers) {}

const ShaderProgram& PointShadowShader::program() const { return _program; }

GLsizei PointShadowShader::layers() const { return _layers; }
