#pragma once

#include "scoped_binding.h"
#include "textures/texture_2d.h"

#include <GL/glew.h>

// Depth-only render target for shadow mapping. The depth texture is what
// the lighting shader samples. Fully set up (or throws) on construction.
class ShadowFrameBuffer {
public:
    ShadowFrameBuffer(GLsizei width, GLsizei height);
    ShadowFrameBuffer(const ShadowFrameBuffer&) = delete;
    ShadowFrameBuffer& operator=(const ShadowFrameBuffer&) = delete;
    ShadowFrameBuffer(ShadowFrameBuffer&& other) noexcept;
    ShadowFrameBuffer& operator=(ShadowFrameBuffer&& other) noexcept;
    ~ShadowFrameBuffer();

    GLuint id() const;
    GLsizei width() const;
    GLsizei height() const;
    const Texture2D& texture() const;

    void bindDraw() const;
    void bindRead() const;
    static void clearDrawBinding();
    static void clearReadBinding();

private:
    void allocateDepthTexture();
    void release() noexcept;

    Texture2D _texture;
    GLuint _id = 0;
    GLsizei _width = 0, _height = 0;
};

using ShadowDrawBindingBase = ScopedBinding<ShadowFrameBuffer, &ShadowFrameBuffer::bindDraw,
                                            &ShadowFrameBuffer::clearDrawBinding>;
using ShadowReadBindingBase = ScopedBinding<ShadowFrameBuffer, &ShadowFrameBuffer::bindRead,
                                            &ShadowFrameBuffer::clearReadBinding>;

// Renders into a shadow map. Also sets the viewport to the shadow map's size
// and restores the previous viewport on exit.
class ShadowDrawBinding : ShadowDrawBindingBase {
public:
    explicit ShadowDrawBinding(const ShadowFrameBuffer& sfbo);
    ~ShadowDrawBinding();

    void attachDepth(const Texture2D& texture);
    void disableDrawBuffer();
    void clearDepth();
    GLenum status() const;

private:
    GLint _previousViewport[4] = {0, 0, 0, 0};
};

// Only used during setup, to tell GL the framebuffer has no colour to read.
class ShadowReadBinding : ShadowReadBindingBase {
public:
    explicit ShadowReadBinding(const ShadowFrameBuffer& sfbo) : ShadowReadBindingBase(sfbo) {}

    void disableReadBuffer();
};
