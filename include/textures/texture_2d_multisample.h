#pragma once

#include "scoped_binding.h"

#include <GL/glew.h>

// Multisampled colour texture, used as a FrameBuffer's colour attachment.
// Has no filters or mipmaps: shaders read individual samples with texelFetch.
class Texture2DMultisample {
public:
    explicit Texture2DMultisample(GLsizei samples, GLenum internalFormat = GL_RGB8);
    Texture2DMultisample(const Texture2DMultisample&) = delete;
    Texture2DMultisample& operator=(const Texture2DMultisample&) = delete;
    Texture2DMultisample(Texture2DMultisample&& other) noexcept;
    Texture2DMultisample& operator=(Texture2DMultisample&& other) noexcept;
    ~Texture2DMultisample();

    // Allocates (or reallocates, e.g. on resize) storage for width x height.
    void createTexture(GLsizei width, GLsizei height);

    GLuint id() const;
    GLsizei samples() const;
    GLenum internalFormat() const;

    void bind() const;
    static void clearBinding();

private:
    void release() noexcept;

    GLuint _id = 0;
    GLsizei _samples;
    GLenum _internalFormat;
};

// Configuration guard. Restoring an outer binding happens on the *active*
// texture unit, so don't create one while assigning units for a draw call.
class Texture2DMultisampleBinding : ScopedBinding<Texture2DMultisample> {
public:
    explicit Texture2DMultisampleBinding(const Texture2DMultisample& texture)
        : ScopedBinding(texture) {}

    void storage(GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height,
                 GLboolean fixedSampleLocations);
};
