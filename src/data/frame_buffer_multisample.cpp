#include "data/frame_buffer_multisample.h"

#include <stb/stb_image_write.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <format>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
    // Saves tightly packed RGB pixels, choosing the format from the file
    // extension.
    void saveImage(const std::filesystem::path& path, GLsizei width,
                   GLsizei height, const unsigned char* pixels) {
        std::string ext = path.extension().string();
        std::transform(
            ext.begin(), ext.end(), ext.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        const std::string file = path.string();

        // OpenGL's origin is bottom-left; image files expect top-left.
        stbi_flip_vertically_on_write(1);

        int ok = 0;
        if (ext == ".png") {
            ok = stbi_write_png(file.c_str(), width, height, 3, pixels,
                                width * 3);
        } else if (ext == ".jpg" || ext == ".jpeg") {
            ok = stbi_write_jpg(file.c_str(), width, height, 3, pixels, 90);
        } else if (ext == ".bmp") {
            ok = stbi_write_bmp(file.c_str(), width, height, 3, pixels);
        } else if (ext == ".tga") {
            ok = stbi_write_tga(file.c_str(), width, height, 3, pixels);
        } else {
            throw std::runtime_error(
                std::format("Unsupported image format: {}", ext));
        }
        if (!ok) {
            throw std::runtime_error(
                std::format("Failed to save image: {}", file));
        }
    }
}  // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FrameBufferMultisample::FrameBufferMultisample(GLsizei width, GLsizei height,
                                               GLsizei samples)
    : _texture(clampSamples(samples), GL_RGBA16F), _width(width),
      _height(height) {
    glGenFramebuffers(1, &_id);
    if (_id == 0)
        throw std::runtime_error("Couldn't make a framebuffer");

    GLenum status;
    {
        DrawFrameBufferBindingMultisample bound(*this);

        _texture.createTexture(_width, _height);
        bound.attachColor(_texture);

        {
            RenderBufferBinding rbo(_rbo);
            rbo.createDepthStencilStorageMultisample(_width, _height,
                                                     _texture.samples());
        }
        bound.attachDepthStencil(_rbo);

        status = bound.status();
    }  // previous draw binding restored here, before any throw

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        // A throwing constructor never runs the destructor, so free the FBO
        // here. _texture and _rbo are fully constructed and clean up after
        // themselves.
        release();
        throw std::runtime_error(
            std::format("Could not complete framebuffer (status {})", status));
    }
}

FrameBufferMultisample::FrameBufferMultisample(
    FrameBufferMultisample&& other) noexcept
    : _texture(std::move(other._texture)), _rbo(std::move(other._rbo)),
      _id(std::exchange(other._id, 0)), _width(other._width),
      _height(other._height) {}

FrameBufferMultisample&
FrameBufferMultisample::operator=(FrameBufferMultisample&& other) noexcept {
    if (this != &other) {
        release();
        _texture = std::move(other._texture);
        _rbo = std::move(other._rbo);
        _id = std::exchange(other._id, 0);
        _width = other._width;
        _height = other._height;
    }
    return *this;
}

FrameBufferMultisample::~FrameBufferMultisample() { release(); }

void FrameBufferMultisample::release() noexcept {
    if (_id != 0) {
        glDeleteFramebuffers(1, &_id);
        _id = 0;
    }
}

GLuint FrameBufferMultisample::id() const { return _id; }

GLsizei FrameBufferMultisample::width() const { return _width; }

GLsizei FrameBufferMultisample::height() const { return _height; }

const Texture2DMultisample& FrameBufferMultisample::texture() const {
    return _texture;
}

void FrameBufferMultisample::bindDraw() const {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _id);
}

void FrameBufferMultisample::bindRead() const {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _id);
}

void FrameBufferMultisample::clearDrawBinding() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void FrameBufferMultisample::clearReadBinding() {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

GLsizei FrameBufferMultisample::clampSamples(GLsizei requested) {
    // 16x MSAA isn't supported everywhere; asking for more than GL_MAX_SAMPLES
    // leaves the framebuffer incomplete.
    GLint maxSamples = 1;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    return std::clamp<GLsizei>(requested, 1, maxSamples);
}

void FrameBufferMultisample::writeToFile(
    const std::filesystem::path& path) const {
    ReadFrameBufferBindingMultisample from(*this);
    auto to = DrawFrameBufferBindingMultisample::screen(_width, _height);
    gl::blit(from, to, _width, _height);

    std::vector<unsigned char> pixels(static_cast<std::size_t>(_width) *
                                      _height * 3);
    {
        auto from = ReadFrameBufferBindingMultisample::screen();
        from.readPixels(_width, _height, GL_RGB, GL_UNSIGNED_BYTE,
                        pixels.data());
    }

    saveImage(path, _width, _height, pixels.data());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DrawFrameBufferBindingMultisample::DrawFrameBufferBindingMultisample(
    const FrameBufferMultisample& fbo)
    : DrawBindingBaseMultisample(fbo) {
    glGetIntegerv(GL_VIEWPORT, _previousViewport);
    glViewport(0, 0, fbo.width(), fbo.height());
}

DrawFrameBufferBindingMultisample::~DrawFrameBufferBindingMultisample() {
    glViewport(_previousViewport[0], _previousViewport[1], _previousViewport[2],
               _previousViewport[3]);
}

DrawFrameBufferBindingMultisample
DrawFrameBufferBindingMultisample::screen(GLsizei width, GLsizei height) {
    return DrawFrameBufferBindingMultisample(nullptr, width, height);
}

void DrawFrameBufferBindingMultisample::attachColor(
    const Texture2DMultisample& texture, GLenum attachment) {
    assertActive();
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment,
                           GL_TEXTURE_2D_MULTISAMPLE, texture.id(), 0);
}

void DrawFrameBufferBindingMultisample::attachDepthStencil(
    const RenderBuffer& rbo) {
    assertActive();
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, rbo.id());
}

void DrawFrameBufferBindingMultisample::clear(const glm::vec4& color,
                                              GLbitfield mask) {
    assertActive();
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(mask);
}

GLenum DrawFrameBufferBindingMultisample::status() const {
    assertActive();
    return glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ReadFrameBufferBindingMultisample ReadFrameBufferBindingMultisample::screen() {
    return ReadFrameBufferBindingMultisample(nullptr);
}

void ReadFrameBufferBindingMultisample::readPixels(GLsizei width,
                                                   GLsizei height,
                                                   GLenum format, GLenum type,
                                                   void* out) const {
    assertActive();
    // Tightly packed rows: without this, any width where a row isn't a multiple
    // of 4 bytes (e.g. RGB with width * 3 % 4 != 0) comes out skewed.
    GLint previousAlignment = 4;
    glGetIntegerv(GL_PACK_ALIGNMENT, &previousAlignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, format, type, out);
    glPixelStorei(GL_PACK_ALIGNMENT, previousAlignment);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void gl::blit(const ReadFrameBufferBindingMultisample& from,
              const DrawFrameBufferBindingMultisample& to, GLsizei width,
              GLsizei height, GLbitfield mask, GLenum filter) {
    from.assertActive();
    to.assertActive();
    // Resolving a multisampled source requires identical source and destination
    // sizes.
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, mask, filter);
}
