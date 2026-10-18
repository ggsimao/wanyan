#include "data/shadow_frame_buffer.h"

#include <glm/vec4.hpp>

#include <stdexcept>
#include <string>
#include <utility>

ShadowFrameBuffer::ShadowFrameBuffer(GLsizei width, GLsizei height)
    : _texture(TextureType::ATTACHMENT), _width(width), _height(height) {
    glGenFramebuffers(1, &_id);
    if (_id == 0) throw std::runtime_error("Couldn't make a shadow framebuffer");

    allocateDepthTexture();

    GLenum status;
    {
        // Both bindings are held at once: in GL 3.3 the read buffer counts
        // towards completeness, so it has to be set before the status check.
        ShadowReadBinding read(*this);
        read.disableReadBuffer();

        ShadowDrawBinding draw(*this);
        draw.attachDepth(_texture);
        draw.disableDrawBuffer();
        status = draw.status();
    }

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        release();
        throw std::runtime_error("Could not complete shadow framebuffer (status " +
                                 std::to_string(status) + ")");
    }
}

ShadowFrameBuffer::ShadowFrameBuffer(ShadowFrameBuffer&& other) noexcept
    : _texture(std::move(other._texture)),
      _id(std::exchange(other._id, 0)),
      _width(other._width),
      _height(other._height) {}

ShadowFrameBuffer& ShadowFrameBuffer::operator=(ShadowFrameBuffer&& other) noexcept {
    if (this != &other) {
        release();
        _texture = std::move(other._texture);
        _id = std::exchange(other._id, 0);
        _width = other._width;
        _height = other._height;
    }
    return *this;
}

ShadowFrameBuffer::~ShadowFrameBuffer() {
    release();
}

void ShadowFrameBuffer::release() noexcept {
    if (_id != 0) {
        glDeleteFramebuffers(1, &_id);
        _id = 0;
    }
}

GLuint ShadowFrameBuffer::id() const {
    return _id;
}

GLsizei ShadowFrameBuffer::width() const {
    return _width;
}

GLsizei ShadowFrameBuffer::height() const {
    return _height;
}

const Texture2D& ShadowFrameBuffer::texture() const {
    return _texture;
}

void ShadowFrameBuffer::bindDraw() const {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _id);
}

void ShadowFrameBuffer::bindRead() const {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _id);
}

void ShadowFrameBuffer::clearDrawBinding() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void ShadowFrameBuffer::clearReadBinding() {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void ShadowFrameBuffer::allocateDepthTexture() {
    Texture2DBinding tex(_texture);
    tex.image(GL_DEPTH_COMPONENT24, _width, _height, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    tex.setFilters(GL_LINEAR, GL_LINEAR);
    // Outside the light's frustum, depth reads as 1.0 (farthest): no shadow.
    tex.setWrapping(GL_CLAMP_TO_BORDER);
    tex.setBorderColor(glm::vec4(1.0f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ShadowDrawBinding::ShadowDrawBinding(const ShadowFrameBuffer& sfbo)
    : ShadowDrawBindingBase(sfbo) {
    glGetIntegerv(GL_VIEWPORT, _previousViewport);
    glViewport(0, 0, sfbo.width(), sfbo.height());
}

ShadowDrawBinding::~ShadowDrawBinding() {
    // Runs before the base destructor restores the previous framebuffer.
    glViewport(_previousViewport[0], _previousViewport[1], _previousViewport[2],
               _previousViewport[3]);
}

void ShadowDrawBinding::attachDepth(const Texture2D& texture) {
    assertActive();
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           texture.id(), 0);
}

void ShadowDrawBinding::disableDrawBuffer() {
    assertActive();
    glDrawBuffer(GL_NONE);
}

void ShadowDrawBinding::clearDepth() {
    assertActive();
    glClear(GL_DEPTH_BUFFER_BIT);
}

GLenum ShadowDrawBinding::status() const {
    assertActive();
    return glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShadowReadBinding::disableReadBuffer() {
    assertActive();
    glReadBuffer(GL_NONE);
}
