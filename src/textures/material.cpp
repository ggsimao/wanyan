#include "textures/material.h"

#include <utility>

Material::Material(std::vector<TexturePtr> diff, std::vector<TexturePtr> spec, float shininess)
    : _diffuseMaps(std::move(diff)), _specularMaps(std::move(spec)), _shininess(shininess) {
    if (_diffuseMaps.empty()) {
        _diffuseMaps.push_back(Texture2D::empty());
    }
    if (_specularMaps.empty()) {
        _specularMaps.push_back(Texture2D::empty());
    }
}

const std::vector<TexturePtr>& Material::diffuseMaps() const {
    return _diffuseMaps;
}

const std::vector<TexturePtr>& Material::specularMaps() const {
    return _specularMaps;
}

float Material::shininess() const {
    return _shininess;
}
