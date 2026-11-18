#include "drawables/basic_mesh.h"

#include "drawables/geometry.h"
#include "drawables/mesh_buffers.h"
#include "drawables/vertex.h"
#include "shaders/shader_program.h"
#include "textures/draw_binding.h"

#include <memory>
#include <utility>

BasicMesh::BasicMesh(const std::vector<Vertex>& vertices,
                     const std::vector<GLuint>& indices, Material material)
    : _buffers(std::make_shared<const MeshBuffers>(
          vertices, indices,
          std::initializer_list<VertexAttribute>{
              VERTEX_ATTRIBUTES[0], VERTEX_ATTRIBUTES[1], VERTEX_ATTRIBUTES[2],
              VERTEX_ATTRIBUTES[3]})),
      _material(std::move(material)) {}

BasicMesh::BasicMesh(std::shared_ptr<const MeshBuffers> buffers,
                     Material material)
    : _buffers(buffers), _material(std::move(material)) {}

BasicMesh BasicMesh::cube(float side, Material material,
                          glm::vec2 texture_scale) {
    const Geometry cube = cubeGeometry(side, texture_scale);
    return BasicMesh(cube.vertices, cube.indices, std::move(material));
}

BasicMesh BasicMesh::square(float side, Material material,
                            glm::vec2 texture_scale) {
    const Geometry square = squareGeometry(side, texture_scale);
    return BasicMesh(square.vertices, square.indices, std::move(material));
}

void BasicMesh::draw(const ShaderProgramBinding& shader,
                     TextureUnits& units) const {
    MaterialDrawBinding material(units, _material);
    shader.setMaterial("material", material);
    _buffers->draw();
}

void BasicMesh::drawPlain(const ShaderProgramBinding& /*shader*/) const {
    _buffers->draw();
}

void BasicMesh::drawVertices(const ShaderProgramBinding& /*shader*/) const {
    _buffers->drawVertices();
}

void BasicMesh::drawPositions(const ShaderProgramBinding& /*shader*/) const {
    _buffers->drawPositions();
}

void BasicMesh::drawInstanced(const ShaderProgramBinding& shader,
                              TextureUnits& units, GLsizei instances) const {
    MaterialDrawBinding material(units, _material);
    shader.setMaterial("material", material);
    _buffers->drawInstanced(instances);
}

void BasicMesh::drawPlainInstanced(const ShaderProgramBinding& /*shader*/,
                                   GLsizei instances) const {
    _buffers->drawInstanced(instances);
}

void BasicMesh::drawVerticesInstanced(const ShaderProgramBinding& /*shader*/,
                                      GLsizei instances) const {
    _buffers->drawVerticesInstanced(instances);
}

void BasicMesh::drawPositionsInstanced(const ShaderProgramBinding& /*shader*/,
                                       GLsizei instances,
                                       GLsizei layers) const {
    _buffers->drawPositionsInstanced(instances, layers);
}

void BasicMesh::bindInstanceBuffer(const ArrayBuffer& instanceBuffer) const {
    _buffers->bindInstanceBuffer(instanceBuffer);
}

const std::shared_ptr<const MeshBuffers>& BasicMesh::buffers() const {
    return _buffers;
}

const Material& BasicMesh::material() const { return _material; }
