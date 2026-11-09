#include "textures/draw_binding.h"
#include "textures/texture_2d.h"

TextureUnits::TextureUnits() {
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &_max);
}

GLint TextureUnits::reserve(GLint count) {
    assert(_next + count <= _max && "Out of texture units");
    const GLint first = _next;
    _next += count;
    return first;
}

void TextureUnits::release(GLint first, [[maybe_unused]] GLint count) {
    assert(_next == first + count &&
           "Texture draw bindings released out of order");
    _next = first;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {
    void bindToUnit(GLint unit, const Texture2D &texture) {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(unit));
        texture.bind();
    }

    inline constexpr GLuint UNIQUE_TEXTURES = 2;
    inline constexpr GLuint NORMAL_OFFSET = 0;
    inline constexpr GLuint DEPTH_OFFSET = 1;
}  // namespace

MaterialDrawBinding::MaterialDrawBinding(TextureUnits &units,
                                         const Material &material)
    : _units(units), _material(material),
      _diffuseCount(static_cast<GLint>(material.diffuseMaps().size())),
      _specularCount(static_cast<GLint>(material.specularMaps().size())),
      _count(_diffuseCount + _specularCount + UNIQUE_TEXTURES),
      _first(units.reserve(_count)) {
    GLint unit = _first;
    for (const TexturePtr &map : material.diffuseMaps()) {
        bindToUnit(unit++, *map);
    }
    for (const TexturePtr &map : material.specularMaps()) {
        bindToUnit(unit++, *map);
    }
    bindToUnit(unit++, *material.normalMap());
    bindToUnit(unit++, *material.depthMap());
}

MaterialDrawBinding::~MaterialDrawBinding() { _units.release(_first, _count); }

const Material &MaterialDrawBinding::material() const { return _material; }

GLint MaterialDrawBinding::diffuseUnit(std::size_t i) const {
    assert(static_cast<GLint>(i) < _diffuseCount &&
           "Diffuse map index out of range");
    return _first + static_cast<GLint>(i);
}

GLint MaterialDrawBinding::specularUnit(std::size_t i) const {
    assert(static_cast<GLint>(i) < _count - _diffuseCount &&
           "Specular map index out of range");
    return _first + _diffuseCount + static_cast<GLint>(i);
}

GLint MaterialDrawBinding::normalUnit() const {
    return _first + _diffuseCount + _specularCount + NORMAL_OFFSET;
}

GLint MaterialDrawBinding::depthUnit() const {
    return _first + _diffuseCount + _specularCount + DEPTH_OFFSET;
}
