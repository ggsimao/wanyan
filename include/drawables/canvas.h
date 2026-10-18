#pragma once

#include "drawable.h"
#include "mesh_buffers.h"
#include "textures/texture_2d_multisample.h"

// A quad covering the whole of clip space, for drawing a screen texture.
// Positions at location 0, texture coordinates at location 1.
class Canvas : public Drawable {
public:
    explicit Canvas(const Texture2DMultisample& texture);

    void draw(const ShaderProgramBinding& shader, TextureUnits& units) const override;
    void drawPlain(const ShaderProgramBinding& shader) const override;

private:
    MeshBuffers _buffers;
    const Texture2DMultisample* _texture;
};
