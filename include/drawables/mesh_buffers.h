#pragma once

#include "data/buffer.h"
#include "data/vertex_array.h"
#include "drawables/vertex.h"

#include <GL/glew.h>

#include <array>
#include <initializer_list>
#include <vector>

// The GPU side of an indexed mesh: VAO, vertex buffer and index buffer,
// set up once on construction. Shared by BasicMesh, Skybox and Canvas.
class MeshBuffers {
public:
    MeshBuffers(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices,
                std::initializer_list<VertexAttribute> attributes);

    void draw() const;
    void drawVertices() const;
    void drawPositions() const;
    void drawInstanced(GLsizei instances) const;
    void drawVerticesInstanced(GLsizei instances) const;
    void drawPositionsInstanced(GLsizei instances, GLsizei layers) const;

    // Reads per-instance model and normal matrices (see Instance) from the
    // given buffer, at attribute locations 3-9.
    void setupInstanceAttributes(const ArrayBuffer& instanceBuffer) const;

    GLsizei indexCount() const;

private:
    VertexArray _vao;
    std::array<ArrayBuffer, VERTEX_ATTRIBUTES.size()> _streams;
    ElementArrayBuffer _ebo;
    GLsizei _indexCount, _vertexCount;
};
