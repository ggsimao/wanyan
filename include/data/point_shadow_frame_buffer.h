#pragma once

#include "scoped_binding.h"
#include "textures/cube_map.h"

#include <GL/glew.h>

// Depth-only render target for shadow mapping with cubemap. The depth texture is what
// the lighting shader samples. Fully set up (or throws) on construction.
class PointShadowFrameBuffer {
public:
    PointShadowFrameBuffer(GLsizei width, GLsizei height);
    PointShadowFrameBuffer(const PointShadowFrameBuffer&) = delete;
    PointShadowFrameBuffer& operator=(const PointShadowFrameBuffer&) = delete;
    PointShadowFrameBuffer(PointShadowFrameBuffer&& other) noexcept;
    PointShadowFrameBuffer& operator=(PointShadowFrameBuffer&& other) noexcept;
    ~PointShadowFrameBuffer();

    GLuint id() const;
    GLsizei width() const;
    GLsizei height() const;
    const CubeMap& texture() const;

    void bindDraw() const;
    void bindRead() const;
    static void clearDrawBinding();
    static void clearReadBinding();

private:
    void allocateDepthTexture();
    void release() noexcept;

    CubeMap _texture;
    GLuint _id = 0;
    GLsizei _width = 0, _height = 0;
};

using PointShadowDrawBindingBase = ScopedBinding<PointShadowFrameBuffer, &PointShadowFrameBuffer::bindDraw,
                                                 &PointShadowFrameBuffer::clearDrawBinding>;
using PointShadowReadBindingBase = ScopedBinding<PointShadowFrameBuffer, &PointShadowFrameBuffer::bindRead,
                                                 &PointShadowFrameBuffer::clearReadBinding>;

// Renders into a shadow map. Also sets the viewport to the shadow map's size
// and restores the previous viewport on exit.
class PointShadowDrawBinding : PointShadowDrawBindingBase {
public:
    explicit PointShadowDrawBinding(const PointShadowFrameBuffer& sfbo);
    ~PointShadowDrawBinding();

    void attachDepth(const CubeMap& texture);
    void disableDrawBuffer();
    void clearDepth();
    GLenum status() const;

private:
    GLint _previousViewport[4] = {0, 0, 0, 0};
};

// Only used during setup, to tell GL the framebuffer has no colour to read.
class PointShadowReadBinding : PointShadowReadBindingBase {
public:
    explicit PointShadowReadBinding(const PointShadowFrameBuffer& sfbo) : PointShadowReadBindingBase(sfbo) {}

    void disableReadBuffer();
};
