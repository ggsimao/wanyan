#include "textures/texture_2d_multisample.h"

#include <stdexcept>
#include <utility>

Texture2DMultisample::Texture2DMultisample(GLsizei samples, GLenum internalFormat)
    : _samples(samples), _internalFormat(internalFormat) {
    glGenTextures(1, &_id);
    if (_id == 0) throw std::runtime_error("Couldn't make a multisample texture");
}

Texture2DMultisample::Texture2DMultisample(Texture2DMultisample&& other) noexcept
    : _id(std::exchange(other._id, 0)),
      _samples(other._samples),
      _internalFormat(other._internalFormat) {}

Texture2DMultisample& Texture2DMultisample::operator=(Texture2DMultisample&& other) noexcept {
    if (this != &other) {
        release();
        _id = std::exchange(other._id, 0);
        _samples = other._samples;
        _internalFormat = other._internalFormat;
    }
    return *this;
}

Texture2DMultisample::~Texture2DMultisample() {
    release();
}

void Texture2DMultisample::release() noexcept {
    if (_id != 0) {
        glDeleteTextures(1, &_id);
        _id = 0;
    }
}

void Texture2DMultisample::createTexture(GLsizei width, GLsizei height) {
    Texture2DMultisampleBinding tex(*this);
    // Fixed sample locations must be GL_TRUE: the framebuffer also has a
    // multisampled renderbuffer attached, and mixing the two is only complete
    // when every texture attachment uses fixed locations.
    tex.storage(_samples, _internalFormat, width, height, GL_TRUE);
}

GLuint Texture2DMultisample::id() const {
    return _id;
}

GLsizei Texture2DMultisample::samples() const {
    return _samples;
}

GLenum Texture2DMultisample::internalFormat() const {
    return _internalFormat;
}

void Texture2DMultisample::bind() const {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _id);
}

void Texture2DMultisample::clearBinding() {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Texture2DMultisampleBinding::storage(GLsizei samples, GLenum internalFormat, GLsizei width,
                                          GLsizei height, GLboolean fixedSampleLocations) {
    assertActive();
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height,
                            fixedSampleLocations);
}
