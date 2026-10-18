#pragma once

#include "textures/cube_map.h"
#include "drawables/drawable.h"
#include "drawables/mesh_buffers.h"

// A cube around the camera, textured with a cube map. Draw it with face
// culling disabled (it's seen from inside), the view matrix's translation
// removed, and depth testing set to GL_LEQUAL.
class Skybox : public Drawable {
public:
    explicit Skybox(CubeMap texture, float size = 10.0f);

    void draw(const ShaderProgramBinding& shader, TextureUnits& units) const override;

    const CubeMap& texture() const;

private:
    MeshBuffers _buffers;
    CubeMap _texture;
};
