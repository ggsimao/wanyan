#include "data/render_buffer.h"
#include <cassert>
#include <stdexcept>
#include <utility>

RenderBuffer::RenderBuffer() {
    glGenRenderbuffers(1, &_id);
    if (_id == 0) throw std::runtime_error("Couldn't make a RBO");
}

RenderBuffer::RenderBuffer(RenderBuffer&& other) noexcept : _id(std::exchange(other._id, 0)) {}

RenderBuffer& RenderBuffer::operator=(RenderBuffer&& other) noexcept {
    if (this != &other) {
        release();
        _id = std::exchange(other._id, 0);
    }

    return *this;
}

RenderBuffer::~RenderBuffer() {
    release();
}

void RenderBuffer::release() noexcept {
    if (_id != 0) {
        glDeleteRenderbuffers(1, &_id);
        _id = 0;
    }
}

GLuint RenderBuffer::id() const {
    return _id;
}

void RenderBuffer::bind() const {
    glBindRenderbuffer(GL_RENDERBUFFER, _id);
}

void RenderBuffer::clearBinding() {
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderBufferBinding::createDepthStencilStorage(GLsizei window_width, GLsizei window_height) {
    assertActive();
    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH24_STENCIL8,
        window_width,
        window_height
    );
}

void RenderBufferBinding::createDepthStencilStorageMultisample(
    GLsizei width, GLsizei height, GLsizei samples
) {
    assertActive();
    glRenderbufferStorageMultisample(
        GL_RENDERBUFFER,
        samples,
        GL_DEPTH24_STENCIL8,
        width,
        height
    );
}
