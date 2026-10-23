#include "drawables/skybox.h"

#include "textures/draw_binding.h"
#include "drawables/geometry.h"
#include "shaders/shader_program.h"

#include <cstddef>
#include <utility>

namespace {
    MeshBuffers makeSkyboxBuffers(float size) {
        const Geometry cube = cubeGeometry(size);
        // The skybox shader only reads positions: the direction from the centre
        // is the cube map lookup vector.
        return MeshBuffers(cube.vertices, cube.indices, {{0, offsetof(Vertex, pos)}});
    }
}  // namespace

Skybox::Skybox(CubeMap texture, float size)
    : _buffers(makeSkyboxBuffers(size)), _texture(std::move(texture)) {}

void Skybox::draw(const ShaderProgramBinding& shader, TextureUnits& units) const {
    TextureDrawBinding cube(units, _texture);
    shader.setTexture("skybox", cube);
    _buffers.draw();
}

void Skybox::drawPlain(const ShaderProgramBinding& /*shader*/) const {
    _buffers.draw();
}

void Skybox::drawVertices(const ShaderProgramBinding& /*shader*/) const {
    _buffers.drawVertices();
}

const CubeMap& Skybox::texture() const {
    return _texture;
}
