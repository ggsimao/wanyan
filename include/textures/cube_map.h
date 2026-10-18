#pragma once

#include "scoped_binding.h"
#include "helpers.h"

#include <GL/glew.h>

#include <array>
#include <filesystem>

// Six square faces, in OpenGL's order: +X, -X, +Y, -Y, +Z, -Z
// (right, left, top, bottom, front, back).
using CubeMapPaths = std::array<std::filesystem::path, 6>;

class CubeMap {
public:
    explicit CubeMap(TextureType type);
    // Loads the six faces and sets the wrapping mode.
    CubeMap(TextureType type, const CubeMapPaths& paths, GLenum wrapping);
    CubeMap(const CubeMap&) = delete;
    CubeMap& operator=(const CubeMap&) = delete;
    CubeMap(CubeMap&& other) noexcept;
    CubeMap& operator=(CubeMap&& other) noexcept;
    ~CubeMap();

    GLuint id() const;
    TextureType type() const;

    void bind() const;
    static void clearBinding();

private:
    void release() noexcept;
    void load(const CubeMapPaths& paths);

    GLuint _id = 0;
    TextureType _type;
};

// Configuration guard: upload faces and set parameters while bound.
// Restoring an outer binding happens on the *active* texture unit, so don't
// create one while texture units are being assigned for a draw call.
class CubeMapBinding : ScopedBinding<CubeMap> {
public:
    explicit CubeMapBinding(const CubeMap& cubeMap) : ScopedBinding(cubeMap) {}

    // face: 0..5, in the order documented on CubeMapPaths.
    void image(unsigned face, GLint internalFormat, GLsizei width, GLsizei height,
               GLenum format, GLenum type, const void* data);
    void generateMipmap();
    void setFilters(GLenum minFilter, GLenum magFilter);
    void setWrapping(GLenum wrapping);
    void setWrappingOnAxis(GLenum axis, GLenum wrapping);
};
