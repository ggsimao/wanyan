#pragma once

#include "textures/texture_2d.h"

#include <vector>

// Diffuse and specular maps plus shininess. Always holds at least one map of
// each kind: missing ones are filled with Texture2D::empty().
class Material {
public:
    Material(std::vector<TexturePtr> diff, std::vector<TexturePtr> spec, float shininess);

    const std::vector<TexturePtr>& diffuseMaps() const;
    const std::vector<TexturePtr>& specularMaps() const;
    float shininess() const;

private:
    std::vector<TexturePtr> _diffuseMaps, _specularMaps;
    float _shininess;
};
