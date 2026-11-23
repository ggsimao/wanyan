#pragma once

#include "data/render_buffer.h"
#include "textures/texture_2d_multisample.h"

#include <GL/glew.h>
#include <filesystem>
#include <glm/vec4.hpp>

inline constexpr GLsizei DEFAULT_SAMPLES = 4;

// Multisampled offscreen render target: a colour texture plus a depth/stencil
// renderbuffer. Fully set up (or throws) on construction.
class FrameBufferMultisample {
  public:
    FrameBufferMultisample(GLsizei width, GLsizei height,
                           GLsizei samples = DEFAULT_SAMPLES);
    FrameBufferMultisample(const FrameBufferMultisample&) = delete;
    FrameBufferMultisample& operator=(const FrameBufferMultisample&) = delete;
    FrameBufferMultisample(FrameBufferMultisample&& other) noexcept;
    FrameBufferMultisample& operator=(FrameBufferMultisample&& other) noexcept;
    ~FrameBufferMultisample();

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
    // Blits, reads the window's framebuffer back and saves it (.png, .jpg,
    // .bmp, .tga).
    void writeToFile(const std::filesystem::path& path) const;

  private:
    static GLsizei clampSamples(GLsizei requested);
    void release() noexcept;

    Texture2DMultisample _texture;
    RenderBuffer _rbo;
    GLuint _id = 0;
    GLsizei _width = 0, _height = 0;
};

using DrawBindingBaseMultisample =
    ScopedBinding<FrameBufferMultisample, &FrameBufferMultisample::bindDraw,
                  &FrameBufferMultisample::clearDrawBinding>;
using ReadBindingBaseMultisample =
    ScopedBinding<FrameBufferMultisample, &FrameBufferMultisample::bindRead,
                  &FrameBufferMultisample::clearReadBinding>;

// Where rendering goes: draw calls, clears, attachments, completeness.
class DrawFrameBufferBindingMultisample : DrawBindingBaseMultisample {
  public:
    explicit DrawFrameBufferBindingMultisample(
        const FrameBufferMultisample& fbo);
    ~DrawFrameBufferBindingMultisample();
    [[nodiscard]] static DrawFrameBufferBindingMultisample
    screen(GLsizei width, GLsizei height);

    void attachColor(const Texture2DMultisample& texture,
                     GLenum attachment = GL_COLOR_ATTACHMENT0);
    void attachDepthStencil(const RenderBuffer& rbo);
    void clear(const glm::vec4& color, GLbitfield mask = GL_COLOR_BUFFER_BIT |
                                                         GL_DEPTH_BUFFER_BIT |
                                                         GL_STENCIL_BUFFER_BIT);
    GLenum status() const;

    using DrawBindingBaseMultisample::assertActive;

  private:
    explicit DrawFrameBufferBindingMultisample(std::nullptr_t, GLsizei width,
                                               GLsizei height)
        : DrawBindingBaseMultisample(nullptr) {
        glGetIntegerv(GL_VIEWPORT, _previousViewport);
        glViewport(0, 0, width, height);
    }

    GLint _previousViewport[4] = {0, 0, 0, 0};
};

// Where pixels come from: glReadPixels and the source side of a blit.
class ReadFrameBufferBindingMultisample : ReadBindingBaseMultisample {
  public:
    explicit ReadFrameBufferBindingMultisample(
        const FrameBufferMultisample& fbo)
        : ReadBindingBaseMultisample(fbo) {}
    [[nodiscard]] static ReadFrameBufferBindingMultisample screen();

    void readPixels(GLsizei width, GLsizei height, GLenum format, GLenum type,
                    void* out) const;

    using ReadBindingBaseMultisample::assertActive;

  private:
    explicit ReadFrameBufferBindingMultisample(std::nullptr_t)
        : ReadBindingBaseMultisample(nullptr) {}
};

namespace gl {
    // Copies a width x height rectangle from the read binding to the draw
    // binding. Taking both bindings proves both framebuffers are bound.
    void blit(const ReadFrameBufferBindingMultisample& from,
              const DrawFrameBufferBindingMultisample& to, GLsizei width,
              GLsizei height, GLbitfield mask = GL_COLOR_BUFFER_BIT,
              GLenum filter = GL_LINEAR);
}  // namespace gl
