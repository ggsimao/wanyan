#include "data/frame_buffer.h"

#include "stb/stb_image_write.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
    // Saves tightly packed RGB pixels, choosing the format from the file extension.
    void saveImage(const std::filesystem::path& path, GLsizei width, GLsizei height,
                const unsigned char* pixels) {
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        const std::string file = path.string();

        // OpenGL's origin is bottom-left; image files expect top-left.
        stbi_flip_vertically_on_write(1);

        int ok = 0;
        if (ext == ".png") {
            ok = stbi_write_png(file.c_str(), width, height, 3, pixels, width * 3);
        } else if (ext == ".jpg" || ext == ".jpeg") {
            ok = stbi_write_jpg(file.c_str(), width, height, 3, pixels, 90);
        } else if (ext == ".bmp") {
            ok = stbi_write_bmp(file.c_str(), width, height, 3, pixels);
        } else if (ext == ".tga") {
            ok = stbi_write_tga(file.c_str(), width, height, 3, pixels);
        } else {
            throw std::runtime_error("Unsupported image format: " + ext);
        }
        if (!ok) {
            throw std::runtime_error("Failed to save image: " + file);
        }
    }
}  // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FrameBuffer::FrameBuffer(GLsizei width, GLsizei height, GLsizei samples)
    : _texture(clampSamples(samples)), _width(width), _height(height) {
    glGenFramebuffers(1, &_id);
    if (_id == 0) throw std::runtime_error("Couldn't make a framebuffer");

    GLenum status;
    {
        DrawFrameBufferBinding bound(*this);

        _texture.createTexture(_width, _height);
        bound.attachColor(_texture);

        {
            RenderBufferBinding rbo(_rbo);
            rbo.createDepthStencilStorageMultisample(_width, _height, _texture.samples());
        }
        bound.attachDepthStencil(_rbo);

        status = bound.status();
    }  // previous draw binding restored here, before any throw

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        // A throwing constructor never runs the destructor, so free the FBO here.
        // _texture and _rbo are fully constructed and clean up after themselves.
        release();
        throw std::runtime_error("Could not complete framebuffer (status " +
                                 std::to_string(status) + ")");
    }
}

FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
    : _texture(std::move(other._texture)),
      _rbo(std::move(other._rbo)),
      _id(std::exchange(other._id, 0)),
      _width(other._width),
      _height(other._height) {}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
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

FrameBuffer::~FrameBuffer() {
    release();
}

void FrameBuffer::release() noexcept {
    if (_id != 0) {
        glDeleteFramebuffers(1, &_id);
        _id = 0;
    }
}

GLuint FrameBuffer::id() const {
    return _id;
}

GLsizei FrameBuffer::width() const {
    return _width;
}

GLsizei FrameBuffer::height() const {
    return _height;
}

const Texture2DMultisample& FrameBuffer::texture() const {
    return _texture;
}

void FrameBuffer::bindDraw() const {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _id);
}

void FrameBuffer::bindRead() const {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _id);
}

void FrameBuffer::clearDrawBinding() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void FrameBuffer::clearReadBinding() {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

GLsizei FrameBuffer::clampSamples(GLsizei requested) {
    // 16x MSAA isn't supported everywhere; asking for more than GL_MAX_SAMPLES
    // leaves the framebuffer incomplete.
    GLint maxSamples = 1;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    return std::clamp<GLsizei>(requested, 1, maxSamples);
}

void FrameBuffer::writeToFile(const std::filesystem::path& path) const {
    ReadFrameBufferBinding from(*this);
    auto to = DrawFrameBufferBinding::screen();
    gl::blit(from, to, _width, _height);

    std::vector<unsigned char> pixels(static_cast<std::size_t>(_width) * _height * 3);
    {
        auto from = ReadFrameBufferBinding::screen();
        from.readPixels(_width, _height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    }

    saveImage(path, _width, _height, pixels.data());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DrawFrameBufferBinding DrawFrameBufferBinding::screen() {
    return DrawFrameBufferBinding(nullptr);
}

void DrawFrameBufferBinding::attachColor(const Texture2DMultisample& texture, GLenum attachment) {
    assertActive();
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, GL_TEXTURE_2D_MULTISAMPLE,
                           texture.id(), 0);
}

void DrawFrameBufferBinding::attachDepthStencil(const RenderBuffer& rbo) {
    assertActive();
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              rbo.id());
}

void DrawFrameBufferBinding::clear(const glm::vec4& color, GLbitfield mask) {
    assertActive();
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(mask);
}

GLenum DrawFrameBufferBinding::status() const {
    assertActive();
    return glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ReadFrameBufferBinding ReadFrameBufferBinding::screen() {
    return ReadFrameBufferBinding(nullptr);
}

void ReadFrameBufferBinding::readPixels(GLsizei width, GLsizei height, GLenum format, GLenum type,
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

void gl::blit(const ReadFrameBufferBinding& from, const DrawFrameBufferBinding& to,
              GLsizei width, GLsizei height, GLbitfield mask, GLenum filter) {
    from.assertActive();
    to.assertActive();
    // Resolving a multisampled source requires identical source and destination sizes.
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, mask, filter);
}
