#pragma once

#include "scoped_binding.h"
#include <GL/glew.h>

class RenderBuffer {
public:
    RenderBuffer();
    RenderBuffer(const RenderBuffer&) = delete;
    RenderBuffer& operator=(const RenderBuffer&) = delete;
    RenderBuffer(RenderBuffer&& other) noexcept;
    RenderBuffer& operator=(RenderBuffer&& other) noexcept;
    ~RenderBuffer();
    GLuint id() const;
    void bind() const;
    static void clearBinding();
private:
    void release() noexcept;
    GLuint _id = 0;
};

class RenderBufferBinding : ScopedBinding<RenderBuffer> {
public:
    explicit RenderBufferBinding(const RenderBuffer& rbo) : ScopedBinding(rbo) {}
    RenderBufferBinding(const RenderBufferBinding&) = delete;
    RenderBufferBinding& operator=(const RenderBufferBinding&) = delete;
    RenderBufferBinding(RenderBufferBinding&&) = delete;
    RenderBufferBinding& operator=(RenderBufferBinding&&) = delete;
    void createDepthStencilStorage(GLsizei window_width, GLsizei window_height);
    void createDepthStencilStorageMultisample(GLsizei width, GLsizei height, GLsizei samples);
};
