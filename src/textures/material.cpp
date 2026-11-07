#include "textures/material.h"
#include "textures/texture_2d.h"

#include <utility>

Material::Material(
    std::vector<TexturePtr> diff, std::vector<TexturePtr> spec, float shininess, TexturePtr normal
) : _diffuseMaps(std::move(diff)), _specularMaps(std::move(spec)), _normalMap(std::move(normal)), _shininess(shininess) {
    if (_diffuseMaps.empty()) {
        _diffuseMaps.push_back(Texture2D::empty());
    }
    if (_specularMaps.empty()) {
        _specularMaps.push_back(Texture2D::empty());
    }
    if (_normalMap == nullptr) {
        _normalMap = Texture2D::flatNormal();
    }
}

const std::vector<TexturePtr>& Material::diffuseMaps() const {
    return _diffuseMaps;
}

const std::vector<TexturePtr>& Material::specularMaps() const {
    return _specularMaps;
}

const TexturePtr Material::normalMap() const {
    return _normalMap;
}

float Material::shininess() const {
    return _shininess;
}
