#pragma once

#include "shaders/shader_program.h"

#include <GL/glew.h>

#include <filesystem>
#include <string>

// A point-shadow program, together with how many layers each instance is
// drawn into. The two must match: a layered program writes gl_Layer from its
// vertex shader and needs every instance drawn once per cube face, while a
// program with a geometry shader emits the faces itself and needs one draw
// per instance. The named constructors set both, so they can't disagree.
class PointShadowShader {
public:
    // gl_Layer written by the vertex shader (ARB_shader_viewport_layer_array,
    // core in OpenGL 4.6): each instance is drawn once per cube face.
    static PointShadowShader layered(std::string label, const std::filesystem::path& vertex,
                                     const std::filesystem::path& fragment);
    // A geometry shader emits all six faces: each instance is drawn once.
    static PointShadowShader withGeometryShader(std::string label,
                                                const std::filesystem::path& vertex,
                                                const std::filesystem::path& geometry,
                                                const std::filesystem::path& fragment);

    const ShaderProgram& program() const;
    GLsizei layers() const;

private:
    PointShadowShader(ShaderProgram program, GLsizei layers);

    ShaderProgram _program;
    GLsizei _layers;
};
