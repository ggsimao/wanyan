#include "drawables/canvas.h"

#include "drawables/geometry.h"

#include <cstddef>

namespace {

MeshBuffers makeCanvasBuffers() {
    const Geometry quad = quadGeometry(2.0f);
    // The screen shader reads texture coordinates at location 1, not 2.
    return MeshBuffers(quad.vertices, quad.indices,
                       {{0, offsetof(Vertex, pos)}, {1, offsetof(Vertex, texCoords)}});
}

}  // namespace

Canvas::Canvas() : _buffers(makeCanvasBuffers()) {}

void Canvas::draw(const ShaderProgramBinding& /*shader*/, TextureUnits& /*units*/) const {
    _buffers.draw();
}
