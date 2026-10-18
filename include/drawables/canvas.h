#pragma once

#include "drawable.h"
#include "mesh_buffers.h"

// A quad covering the whole of clip space, for drawing a screen texture.
// Positions at location 0, texture coordinates at location 1.
//
// It binds no texture itself: the caller binds whatever it is presenting
// (e.g. a FrameBuffer's multisample texture) before calling draw().
class Canvas : public Drawable {
public:
    Canvas();

    void draw(const ShaderProgramBinding& shader, TextureUnits& units) const override;

private:
    MeshBuffers _buffers;
};
