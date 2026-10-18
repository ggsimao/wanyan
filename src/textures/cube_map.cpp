#include "textures/cube_map.h"

#include <stb/stb_image.h>

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
    struct StbiDeleter {
        void operator()(unsigned char* pixels) const { stbi_image_free(pixels); }
    };
    using StbiPixels = std::unique_ptr<unsigned char, StbiDeleter>;
}  // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CubeMap::CubeMap(TextureType type) : _type(type) {
    glGenTextures(1, &_id);
    if (_id == 0) throw std::runtime_error("Couldn't make a cube map");
}

CubeMap::CubeMap(TextureType type, const CubeMapPaths& paths, GLenum wrapping)
    : CubeMap(type) {
    load(paths);
    CubeMapBinding cube(*this);
    cube.setWrapping(wrapping);
}

CubeMap::CubeMap(CubeMap&& other) noexcept
    : _id(std::exchange(other._id, 0)), _type(other._type) {}

CubeMap& CubeMap::operator=(CubeMap&& other) noexcept {
    if (this != &other) {
        release();
        _id = std::exchange(other._id, 0);
        _type = other._type;
    }
    return *this;
}

CubeMap::~CubeMap() {
    release();
}

void CubeMap::release() noexcept {
    if (_id != 0) {
        glDeleteTextures(1, &_id);
        _id = 0;
    }
}

void CubeMap::load(const CubeMapPaths& paths) {
    // stb's flip flag is global and Texture2D::load sets it, so set it
    // explicitly: cube map faces must not be flipped.
    stbi_set_flip_vertically_on_load(0);

    CubeMapBinding cube(*this);
    for (unsigned face = 0; face < paths.size(); ++face) {
        int width = 0, height = 0, channels = 0;
        StbiPixels data(stbi_load(paths[face].string().c_str(), &width, &height, &channels, 0));
        if (!data) {
            throw std::runtime_error("Couldn't load cube map face: " + paths[face].string());
        }
        const GLenum format = channels == 4 ? GL_RGBA : GL_RGB;
        // Skybox images are colour data authored in sRGB.
        cube.image(face, GL_SRGB8, width, height, format, GL_UNSIGNED_BYTE, data.get());
    }
    // Once, after all six faces exist: mipmaps need a complete cube.
    cube.generateMipmap();
}

GLuint CubeMap::id() const {
    return _id;
}

TextureType CubeMap::type() const {
    return _type;
}

void CubeMap::bind() const {
    glBindTexture(GL_TEXTURE_CUBE_MAP, _id);
}

void CubeMap::clearBinding() {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CubeMapBinding::image(unsigned face, GLint internalFormat, GLsizei width, GLsizei height,
                           GLenum format, GLenum type, const void* data) {
    assertActive();
    assert(face < 6 && "A cube map has six faces");
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, internalFormat, width, height, 0,
                 format, type, data);
}

void CubeMapBinding::generateMipmap() {
    assertActive();
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void CubeMapBinding::setFilters(GLenum minFilter, GLenum magFilter) {
    assertActive();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(minFilter));
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(magFilter));
}

void CubeMapBinding::setWrapping(GLenum wrapping) {
    assertActive();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrapping));
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrapping));
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, static_cast<GLint>(wrapping));
}

void CubeMapBinding::setWrappingOnAxis(GLenum axis, GLenum wrapping) {
    assertActive();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, axis, static_cast<GLint>(wrapping));
}
