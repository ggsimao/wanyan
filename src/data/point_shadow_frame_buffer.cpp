#include "data/point_shadow_frame_buffer.h"
#include "textures/cube_map.h"

#include <format>
#include <glm/vec4.hpp>

#include <stdexcept>
#include <utility>

PointShadowFrameBuffer::PointShadowFrameBuffer(GLsizei width, GLsizei height)
    : _texture(TextureType::ATTACHMENT), _width(width), _height(height) {
    glGenFramebuffers(1, &_id);
    if (_id == 0) throw std::runtime_error("Couldn't make a shadow framebuffer");

    allocateDepthTexture();

    GLenum status;
    {
        // Both bindings are held at once: in GL 3.3 the read buffer counts
        // towards completeness, so it has to be set before the status check.
        PointShadowReadBinding read(*this);
        read.disableReadBuffer();

        PointShadowDrawBinding draw(*this);
        draw.attachDepth(_texture);
        draw.disableDrawBuffer();
        status = draw.status();
    }

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        release();
        throw std::runtime_error(std::format("Could not complete shadow framebuffer (status {})",
                                 status));
    }
}

PointShadowFrameBuffer::PointShadowFrameBuffer(PointShadowFrameBuffer&& other) noexcept
    : _texture(std::move(other._texture)),
      _id(std::exchange(other._id, 0)),
      _width(other._width),
      _height(other._height) {}

PointShadowFrameBuffer& PointShadowFrameBuffer::operator=(PointShadowFrameBuffer&& other) noexcept {
    if (this != &other) {
        release();
        _texture = std::move(other._texture);
        _id = std::exchange(other._id, 0);
        _width = other._width;
        _height = other._height;
    }
    return *this;
}

PointShadowFrameBuffer::~PointShadowFrameBuffer() {
    release();
}

void PointShadowFrameBuffer::release() noexcept {
    if (_id != 0) {
        glDeleteFramebuffers(1, &_id);
        _id = 0;
    }
}

GLuint PointShadowFrameBuffer::id() const {
    return _id;
}

GLsizei PointShadowFrameBuffer::width() const {
    return _width;
}

GLsizei PointShadowFrameBuffer::height() const {
    return _height;
}

const CubeMap& PointShadowFrameBuffer::texture() const {
    return _texture;
}

void PointShadowFrameBuffer::bindDraw() const {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _id);
}

void PointShadowFrameBuffer::bindRead() const {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _id);
}

void PointShadowFrameBuffer::clearDrawBinding() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void PointShadowFrameBuffer::clearReadBinding() {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void PointShadowFrameBuffer::allocateDepthTexture() {
    CubeMapBinding tex(_texture);
    for (size_t i = 0; i < 6; i++) {
        tex.image(i, GL_DEPTH_COMPONENT24, _width, _height, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }
    tex.setFilters(GL_NEAREST, GL_NEAREST);
    tex.setWrapping(GL_CLAMP_TO_EDGE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PointShadowDrawBinding::PointShadowDrawBinding(const PointShadowFrameBuffer& sfbo)
    : PointShadowDrawBindingBase(sfbo) {
    glGetIntegerv(GL_VIEWPORT, _previousViewport);
    glViewport(0, 0, sfbo.width(), sfbo.height());
}

PointShadowDrawBinding::~PointShadowDrawBinding() {
    // Runs before the base destructor restores the previous framebuffer.
    glViewport(_previousViewport[0], _previousViewport[1], _previousViewport[2],
               _previousViewport[3]);
}

void PointShadowDrawBinding::attachDepth(const CubeMap& texture) {
    assertActive();
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture.id(), 0);
}

void PointShadowDrawBinding::disableDrawBuffer() {
    assertActive();
    glDrawBuffer(GL_NONE);
}

void PointShadowDrawBinding::clearDepth() {
    assertActive();
    glClear(GL_DEPTH_BUFFER_BIT);
}

GLenum PointShadowDrawBinding::status() const {
    assertActive();
    return glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PointShadowReadBinding::disableReadBuffer() {
    assertActive();
    glReadBuffer(GL_NONE);
}
