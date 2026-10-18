#pragma once

#include "drawables/drawable.h"
#include "textures/material.h"
#include "drawables/mesh_buffers.h"
#include "drawables/vertex.h"

#include <GL/glew.h>

#include <vector>

// An indexed triangle mesh with a material: positions, normals and
// texture coordinates at attribute locations 0, 1 and 2.
class BasicMesh : public Drawable {
public:
    BasicMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices,
              Material material);

    // Centred on the origin. Flat-shaded: each face has its own vertices and normal.
    static BasicMesh cube(float side, Material material = Material({}, {}, 1.0f));
    // In the XY plane, centred on the origin, visible from both sides.
    static BasicMesh square(float side, Material material = Material({}, {}, 1.0f));

    void draw(const ShaderProgramBinding& shader, TextureUnits& units) const override;
    void drawPlain(const ShaderProgramBinding& shader) const override;
    void drawInstanced(const ShaderProgramBinding& shader, TextureUnits& units,
                       GLsizei instances) const override;
    void drawPlainInstanced(const ShaderProgramBinding& shader, GLsizei instances) const override;
    void setupInstanceAttributes(const ArrayBuffer& instanceBuffer) const override;

    const Material& material() const;

private:
    MeshBuffers _buffers;
    Material _material;
};
