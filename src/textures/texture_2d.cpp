#include "textures/texture_2d.h"

#include <stb/stb_image.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
    // Owns the pixels returned by stbi_load and frees them however the scope ends.
    struct StbiDeleter {
        void operator()(unsigned char* pixels) const { stbi_image_free(pixels); }
    };
    using StbiPixels = std::unique_ptr<unsigned char, StbiDeleter>;

    inline constexpr unsigned char EMPTY_DATA[4] = {0, 0, 0, 0};
}  // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Texture2D::Texture2D(TextureType type) : _type(type) {
    glGenTextures(1, &_id);
    if (_id == 0) throw std::runtime_error("Couldn't make a texture");
}

Texture2D::Texture2D(TextureType type, const std::filesystem::path& path, GLenum wrapping)
    : Texture2D(type) {
    load(path);
    Texture2DBinding tex(*this);
    tex.setWrapping(wrapping);
}

Texture2D::Texture2D(Texture2D&& other) noexcept
    : _id(std::exchange(other._id, 0)), _type(other._type) {}

Texture2D& Texture2D::operator=(Texture2D&& other) noexcept {
    if (this != &other) {
        release();
        _id = std::exchange(other._id, 0);
        _type = other._type;
    }
    return *this;
}

Texture2D::~Texture2D() {
    release();
}

void Texture2D::release() noexcept {
    if (_id != 0) {
        glDeleteTextures(1, &_id);
        _id = 0;
    }
}

void Texture2D::load(const std::filesystem::path& path) {
    int width = 0, height = 0, channels = 0;
    stbi_set_flip_vertically_on_load(1);
    StbiPixels data(stbi_load(path.string().c_str(), &width, &height, &channels, 0));
    if (!data) {
        throw std::runtime_error("Couldn't load texture: " + path.string());
    }

    const GLenum format = channels == 4 ? GL_RGBA : GL_RGB;
    Texture2DBinding tex(*this);
    tex.image(static_cast<GLint>(internalFormat()), width, height, format,
              GL_UNSIGNED_BYTE, data.get());
    tex.generateMipmap();
}

Texture2D Texture2D::fromColor(TextureType type, const glm::vec3& color) {
    Texture2D texture(type);
    const unsigned char data[4] = {
        static_cast<unsigned char>(color.r * 255.0f),
        static_cast<unsigned char>(color.g * 255.0f),
        static_cast<unsigned char>(color.b * 255.0f),
        255,
    };
    {
        Texture2DBinding tex(texture);
        // Same format as a loaded texture of this type, so a diffuse colour is
        // decoded from sRGB exactly like a diffuse image.
        tex.image(static_cast<GLint>(texture.internalFormat()), 1, 1, GL_RGBA,
                  GL_UNSIGNED_BYTE, data);
    }
    return texture;
}

TexturePtr Texture2D::empty() {
    // Shared by whoever holds it; deleted when the last holder lets go.
    static std::weak_ptr<const Texture2D> cache;
    if (auto existing = cache.lock()) {
        return existing;
    }

    Texture2D texture(TextureType::ATTACHMENT);
    {
        Texture2DBinding tex(texture);
        tex.image(GL_RGBA8, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, EMPTY_DATA);
    }
    auto shared = std::make_shared<const Texture2D>(std::move(texture));
    cache = shared;
    return shared;
}

GLuint Texture2D::id() const {
    return _id;
}

TextureType Texture2D::type() const {
    return _type;
}

GLenum Texture2D::internalFormat() const {
    switch (_type) {
        case TextureType::DIFFUSE:    return GL_SRGB8_ALPHA8;  // colour: stored as sRGB, sampled as linear
        case TextureType::SPECULAR:   return GL_RGBA8;         // data: read as-is
        case TextureType::ATTACHMENT: return GL_RGBA8;
    }
    return GL_RGBA8;  // unreachable; keeps -Wreturn-type quiet
}

void Texture2D::bind() const {
    glBindTexture(GL_TEXTURE_2D, _id);
}

void Texture2D::clearBinding() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Texture2DBinding::image(GLint internalFormat, GLsizei width, GLsizei height,
                             GLenum format, GLenum type, const void* data) {
    assertActive();
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
}

void Texture2DBinding::generateMipmap() {
    assertActive();
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture2DBinding::setFilters(GLenum minFilter, GLenum magFilter) {
    assertActive();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(minFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(magFilter));
}

void Texture2DBinding::setWrapping(GLenum wrapping) {
    assertActive();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrapping));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrapping));
}

void Texture2DBinding::setWrappingOnAxis(GLenum axis, GLenum wrapping) {
    assertActive();
    glTexParameteri(GL_TEXTURE_2D, axis, static_cast<GLint>(wrapping));
}

void Texture2DBinding::setBorderColor(const glm::vec4& color) {
    assertActive();
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &color[0]);
}
