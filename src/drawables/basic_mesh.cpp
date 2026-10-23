#include "drawables/basic_mesh.h"

#include "textures/draw_binding.h"
#include "drawables/geometry.h"
#include "shaders/shader_program.h"

#include <cstddef>
#include <utility>

BasicMesh::BasicMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices,
                     Material material)
    : _buffers(vertices, indices,
               {{0, offsetof(Vertex, pos)},
                {1, offsetof(Vertex, normal)},
                {2, offsetof(Vertex, texCoords)}}),
      _material(std::move(material)) {}

BasicMesh BasicMesh::cube(float side, Material material) {
    const Geometry cube = cubeGeometry(side);
    return BasicMesh(cube.vertices, cube.indices, std::move(material));
}

BasicMesh BasicMesh::square(float side, Material material) {
    const Geometry square = squareGeometry(side);
    return BasicMesh(square.vertices, square.indices, std::move(material));
}

void BasicMesh::draw(const ShaderProgramBinding& shader, TextureUnits& units) const {
    MaterialDrawBinding material(units, _material);
    shader.setMaterial("material", material);
    _buffers.draw();
}

void BasicMesh::drawPlain(const ShaderProgramBinding& /*shader*/) const {
    _buffers.draw();
}

void BasicMesh::drawVertices(const ShaderProgramBinding& /*shader*/) const {
    _buffers.drawVertices();
}

void BasicMesh::drawInstanced(const ShaderProgramBinding& shader, TextureUnits& units,
                              GLsizei instances) const {
    MaterialDrawBinding material(units, _material);
    shader.setMaterial("material", material);
    _buffers.drawInstanced(instances);
}

void BasicMesh::drawPlainInstanced(const ShaderProgramBinding& /*shader*/, GLsizei instances) const {
    _buffers.drawInstanced(instances);
}

void BasicMesh::drawVerticesInstanced(const ShaderProgramBinding& /*shader*/, GLsizei instances) const {
    _buffers.drawVerticesInstanced(instances);
}

void BasicMesh::setupInstanceAttributes(const ArrayBuffer& instanceBuffer) const {
    _buffers.setupInstanceAttributes(instanceBuffer);
}

const Material& BasicMesh::material() const {
    return _material;
}
