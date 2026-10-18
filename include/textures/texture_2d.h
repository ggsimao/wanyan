#pragma once

#include "scoped_binding.h"
#include "textures/helpers.h"

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <memory>

class Texture2D;
using TexturePtr = std::shared_ptr<const Texture2D>;

class Texture2D {
public:
    explicit Texture2D(TextureType type);
    // Loads an image file and sets its wrapping mode.
    Texture2D(TextureType type, const std::filesystem::path& path, GLenum wrapping);
    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;
    Texture2D(Texture2D&& other) noexcept;
    Texture2D& operator=(Texture2D&& other) noexcept;
    ~Texture2D();

    static Texture2D fromColor(TextureType type, const glm::vec3& color);
    // A shared 1x1 transparent black texture, for materials without maps.
    static TexturePtr empty();

    GLuint id() const;
    TextureType type() const;
    GLenum internalFormat() const;

    void bind() const;
    static void clearBinding();

private:
    void release() noexcept;
    void load(const std::filesystem::path& path);

    GLuint _id = 0;
    TextureType _type;
};

// Configuration guard: upload data and set parameters while bound.
// Restoring an outer binding happens on the *active* texture unit, so don't
// create one in the middle of assigning texture units for a draw call.
class Texture2DBinding : ScopedBinding<Texture2D> {
public:
    explicit Texture2DBinding(const Texture2D& texture) : ScopedBinding(texture) {}

    void image(GLint internalFormat, GLsizei width, GLsizei height,
               GLenum format, GLenum type, const void* data);
    void generateMipmap();
    void setFilters(GLenum minFilter, GLenum magFilter);
    void setWrapping(GLenum wrapping);
    void setWrappingOnAxis(GLenum axis, GLenum wrapping);
    void setBorderColor(const glm::vec4& color);
};
