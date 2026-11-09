#pragma once

#include "material.h"

#include <GL/glew.h>

#include <cassert>
#include <cstddef>

// Bindings that attach textures to texture units for sampling in a draw call.
//
// Unlike the configuration guards (Texture2DBinding etc.), these don't restore
// anything on destruction: they only give their units back, and the next
// binding on those units overwrites them. Units must be released in reverse
// order of reservation, which local variables guarantee.
//
//     TextureUnits units;
//     TextureDrawBinding shadow(units, sfbo.texture());        // unit 0
//     shader.setTexture("dirLight.shadow_map", shadow);
//     for (const auto& mesh : meshes) {
//         MaterialDrawBinding material(units, mesh.material()); // units 1..
//         shader.setMaterial("material", material);
//         // draw
//     }

template <typename Texture> class TextureDrawBinding;
class MaterialDrawBinding;

// Hands out texture units in a stack. Create one per render pass, after the
// GL context exists.
class TextureUnits {
  public:
    TextureUnits();
    TextureUnits(const TextureUnits &) = delete;
    TextureUnits &operator=(const TextureUnits &) = delete;
    TextureUnits(TextureUnits &&) = delete;
    TextureUnits &operator=(TextureUnits &&) = delete;

  private:
    template <typename> friend class TextureDrawBinding;
    friend class MaterialDrawBinding;

    GLint reserve(GLint count);
    void release(GLint first, GLint count);

    GLint _next = 0;
    GLint _max = 0;
};

// Binds a sequence of textures (Texture2D, CubeMap or Texture2DMultisample) to
// their own consecutive units.
template <typename Texture> class TextureDrawBinding {
  public:
    TextureDrawBinding(TextureUnits &units, const Texture &texture)
        : _units(units), _first(units.reserve(1)), _count(1) {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(_first));
        texture.bind();
    }
    // Several textures, bound to consecutive units: any sized range whose
    // elements are textures, e.g. a view of the lights' shadow maps.
    template <std::ranges::sized_range R>
    requires std::convertible_to < std::ranges::range_reference_t<R>,
    const Texture & > TextureDrawBinding(TextureUnits &units, R &&textures)
        : _units(units),
    _first(units.reserve(static_cast<GLint>(std::ranges::size(textures)))),
    _count(static_cast<GLint>(std::ranges::size(textures))) {
        GLint unit = _first;
        for (const Texture &texture : textures) {
            glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(unit++));
            texture.bind();
        }
    }
    ~TextureDrawBinding() { _units.release(_first, _count); }

    TextureDrawBinding(const TextureDrawBinding &) = delete;
    TextureDrawBinding &operator=(const TextureDrawBinding &) = delete;
    TextureDrawBinding(TextureDrawBinding &&) = delete;
    TextureDrawBinding &operator=(TextureDrawBinding &&) = delete;

    GLint unit(std::size_t i) const {
        assert(static_cast<GLint>(i) < _count && "Texture index out of range");
        return _first + static_cast<GLint>(i);
    }

    GLint size() const { return _count; }

  private:
    TextureUnits &_units;
    GLint _first;
    GLint _count;
};

// Binds all of a material's maps to a contiguous block of units:
// diffuse maps first, then specular maps.
class MaterialDrawBinding {
  public:
    MaterialDrawBinding(TextureUnits &units, const Material &material);
    ~MaterialDrawBinding();

    MaterialDrawBinding(const MaterialDrawBinding &) = delete;
    MaterialDrawBinding &operator=(const MaterialDrawBinding &) = delete;
    MaterialDrawBinding(MaterialDrawBinding &&) = delete;
    MaterialDrawBinding &operator=(MaterialDrawBinding &&) = delete;

    const Material &material() const;
    GLint diffuseUnit(std::size_t i) const;
    GLint specularUnit(std::size_t i) const;
    GLint normalUnit() const;
    GLint depthUnit() const;

  private:
    // Declaration order matters: _first is initialized from _count.
    TextureUnits &_units;
    const Material &_material;
    GLint _diffuseCount;
    GLint _specularCount;
    GLint _count;
    GLint _first;
};
