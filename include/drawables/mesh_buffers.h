#pragma once

#include "data/buffer.h"
#include "data/vertex_array.h"
#include "drawables/vertex.h"

#include <GL/glew.h>

#include <cstddef>
#include <initializer_list>
#include <vector>

// One vertex attribute read from Vertex: its shader location and the offset
// of the field it reads. Every Vertex field is a vec3, so the format is fixed.
struct VertexAttribute {
    GLuint location;
    std::size_t offset;  // offsetof(Vertex, ...)
};

// The GPU side of an indexed mesh: VAO, vertex buffer and index buffer,
// set up once on construction. Shared by BasicMesh, Skybox and Canvas.
class MeshBuffers {
public:
    MeshBuffers(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices,
                std::initializer_list<VertexAttribute> attributes);

    void draw() const;
    void drawInstanced(GLsizei instances) const;

    // Reads per-instance model and normal matrices (see Instance) from the
    // given buffer, at attribute locations 3-9.
    void setupInstanceAttributes(const ArrayBuffer& instanceBuffer) const;

    GLsizei indexCount() const;

private:
    VertexArray _vao;
    ArrayBuffer _abo;
    ElementArrayBuffer _ebo;
    GLsizei _indexCount;
};
