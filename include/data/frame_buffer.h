#pragma once

#include "textures/texture_2d_multisample.h"
#include "data/render_buffer.h"

#include <GL/glew.h>
#include <filesystem>
#include <glm/vec4.hpp>

inline constexpr GLsizei DEFAULT_SAMPLES = 16;

// Multisampled offscreen render target: a colour texture plus a depth/stencil
// renderbuffer. Fully set up (or throws) on construction.
class FrameBuffer {
public:
    FrameBuffer(GLsizei width, GLsizei height, GLsizei samples = DEFAULT_SAMPLES);
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;
    FrameBuffer(FrameBuffer&& other) noexcept;
    FrameBuffer& operator=(FrameBuffer&& other) noexcept;
    ~FrameBuffer();

    GLuint id() const;
    GLsizei width() const;
    GLsizei height() const;
    const Texture2DMultisample& texture() const;

    void bindDraw() const;
    void bindRead() const;
    static void clearDrawBinding();
    static void clearReadBinding();

    // Resolves the multisampled contents into the window's framebuffer.
    void blit() const;
    // Blits, reads the window's framebuffer back and saves it (.png, .jpg, .bmp, .tga).
    void writeToFile(const std::filesystem::path& path) const;

private:
    static GLsizei clampSamples(GLsizei requested);
    void release() noexcept;

    Texture2DMultisample _texture;
    RenderBuffer _rbo;
    GLuint _id = 0;
    GLsizei _width = 0, _height = 0;
};

using DrawBindingBase =
    ScopedBinding<FrameBuffer, &FrameBuffer::bindDraw, &FrameBuffer::clearDrawBinding>;
using ReadBindingBase =
    ScopedBinding<FrameBuffer, &FrameBuffer::bindRead, &FrameBuffer::clearReadBinding>;

// Where rendering goes: draw calls, clears, attachments, completeness.
class DrawFrameBufferBinding : DrawBindingBase {
public:
    explicit DrawFrameBufferBinding(const FrameBuffer& fbo) : DrawBindingBase(fbo) {}
    [[nodiscard]] static DrawFrameBufferBinding screen();

    void attachColor(const Texture2DMultisample& texture, GLenum attachment = GL_COLOR_ATTACHMENT0);
    void attachDepthStencil(const RenderBuffer& rbo);
    void clear(const glm::vec4& color,
               GLbitfield mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    GLenum status() const;

    using DrawBindingBase::assertActive;

private:
    explicit DrawFrameBufferBinding(std::nullptr_t) : DrawBindingBase(nullptr) {}
};

// Where pixels come from: glReadPixels and the source side of a blit.
class ReadFrameBufferBinding : ReadBindingBase {
public:
    explicit ReadFrameBufferBinding(const FrameBuffer& fbo) : ReadBindingBase(fbo) {}
    [[nodiscard]] static ReadFrameBufferBinding screen();

    void readPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, void* out) const;

    using ReadBindingBase::assertActive;

private:
    explicit ReadFrameBufferBinding(std::nullptr_t) : ReadBindingBase(nullptr) {}
};

namespace gl {
    // Copies a width x height rectangle from the read binding to the draw binding.
    // Taking both bindings proves both framebuffers are bound.
    void blit(const ReadFrameBufferBinding& from, const DrawFrameBufferBinding& to,
            GLsizei width, GLsizei height,
            GLbitfield mask = GL_COLOR_BUFFER_BIT, GLenum filter = GL_LINEAR);
}  // namespace gl
