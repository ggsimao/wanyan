#include "drawables/canvas.h"

#include "drawables/geometry.h"
#include "shaders/shader_program.h"
#include "textures/draw_binding.h"

#include <cstddef>

namespace {

MeshBuffers makeCanvasBuffers() {
    const Geometry quad = quadGeometry(2.0f);
    // The screen shader reads texture coordinates at location 1, not 2.
    return MeshBuffers(quad.vertices, quad.indices,
                       {{0, offsetof(Vertex, pos)}, {1, offsetof(Vertex, texCoords)}});
}

}  // namespace

Canvas::Canvas(const Texture2DMultisample& texture) : _buffers(makeCanvasBuffers()), _texture(&texture) {}

void Canvas::draw(const ShaderProgramBinding& shader, TextureUnits& units) const {
    TextureDrawBinding screenTexture(units, *_texture);
    shader.setTexture("screenTexture", screenTexture);
    shader.setInt("sampleCount", _texture->samples());
    _buffers.draw();
}

void Canvas::drawPlain(const ShaderProgramBinding& /*shader*/) const {
    _buffers.draw();
}

void Canvas::drawVertices(const ShaderProgramBinding& /*shader*/) const {
    _buffers.drawVertices();
}
