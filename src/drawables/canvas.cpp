#include "drawables/canvas.h"

#include "drawables/geometry.h"
#include "drawables/vertex.h"
#include "shaders/shader_program.h"
#include "textures/draw_binding.h"

namespace {
    MeshBuffers makeCanvasBuffers() {
        const Geometry quad = quadGeometry(2.0f);
        // The screen shader reads texture coordinates at location 1, not 2.
        return MeshBuffers(quad.vertices, quad.indices,
                           {VERTEX_ATTRIBUTES[POS_LOCATION],
                            VERTEX_ATTRIBUTES[TEX_COORDS_LOCATION]});
    }
}  // namespace

Canvas::Canvas(const Texture2DMultisample& texture)
    : _buffers(makeCanvasBuffers()), _texture(&texture) {}

void Canvas::draw(const ShaderProgramBinding& shader,
                  TextureUnits& units) const {
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

void Canvas::drawPositions(const ShaderProgramBinding& /*shader*/) const {
    _buffers.drawPositions();
}
