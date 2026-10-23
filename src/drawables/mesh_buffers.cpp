#include "drawables/mesh_buffers.h"

#include "scene/instance.h"

MeshBuffers::MeshBuffers(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices,
                         std::initializer_list<VertexAttribute> attributes)
    : _indexCount(static_cast<GLsizei>(indices.size())), _vertexCount(static_cast<GLsizei>(vertices.size())) {
    VertexArrayBinding vao(_vao);
    {
        auto abo = vao.bindABO(_abo);
        abo.bufferData(vertices, GL_STATIC_DRAW);
        for (const VertexAttribute& attribute : attributes) {
            abo.attribPointer(attribute.location, 3, GL_FLOAT, sizeof(Vertex), attribute.offset);
        }
    }
    auto ebo = vao.bindEBO(_ebo);
    ebo.bufferData(indices, GL_STATIC_DRAW);
}

void MeshBuffers::draw() const {
    VertexArrayBinding vao(_vao);
    vao.drawElements(_indexCount);
}

void MeshBuffers::drawVertices() const {
    VertexArrayBinding vao(_vao);
    vao.drawVertices(_vertexCount, GL_POINTS);
}

void MeshBuffers::drawInstanced(GLsizei instances) const {
    VertexArrayBinding vao(_vao);
    vao.drawElementsInstanced(_indexCount, instances);
}

void MeshBuffers::drawVerticesInstanced(GLsizei instances) const {
    VertexArrayBinding vao(_vao);
    vao.drawVerticesInstanced(_vertexCount, GL_POINTS, instances);
}

void MeshBuffers::setupInstanceAttributes(const ArrayBuffer& instanceBuffer) const {
    constexpr GLuint MODEL_LOCATION = 3;   // mat4: 4 consecutive vec4 locations
    constexpr GLuint NORMAL_LOCATION = 7;  // mat3: 3 consecutive vec3 locations
    constexpr auto STRIDE = static_cast<GLsizei>(sizeof(Instance));

    VertexArrayBinding vao(_vao);
    auto ibo = vao.bindABO(instanceBuffer);
    for (GLuint column = 0; column < 4; ++column) {
        ibo.attribPointer(MODEL_LOCATION + column, 4, GL_FLOAT, STRIDE,
                          Instance::modelOffset() + column * sizeof(glm::vec4));
        ibo.attribDivisor(MODEL_LOCATION + column, 1);  // advance once per instance
    }
    for (GLuint column = 0; column < 3; ++column) {
        ibo.attribPointer(NORMAL_LOCATION + column, 3, GL_FLOAT, STRIDE,
                          Instance::normalOffset() + column * sizeof(glm::vec3));
        ibo.attribDivisor(NORMAL_LOCATION + column, 1);
    }
}

GLsizei MeshBuffers::indexCount() const {
    return _indexCount;
}
