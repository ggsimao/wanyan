#include "textures/material.h"
#include "textures/texture_2d.h"

#include <utility>

Material::Material(MaterialParameters params)
    : _diffuseMaps(std::move(params.diff)),
      _specularMaps(std::move(params.spec)),
      _normalMap(std::move(params.normalMap)),
      _depthMap(std::move(params.depthMap)), _shininess(params.shininess),
      _heightScale(params.heightScale),
      _clipParallaxEdges(params.clipParallaxEdges),
      _hasNormalMap(_normalMap != nullptr) {
    if (_diffuseMaps.empty()) {
        _diffuseMaps.push_back(Texture2D::empty());
    }
    if (_specularMaps.empty()) {
        _specularMaps.push_back(Texture2D::empty());
    }
    if (_normalMap == nullptr) {
        _normalMap = Texture2D::flatNormal();
    }
    if (_depthMap == nullptr) {
        _depthMap = Texture2D::flatDepth();
    }
}

const std::vector<TexturePtr> &Material::diffuseMaps() const {
    return _diffuseMaps;
}

const std::vector<TexturePtr> &Material::specularMaps() const {
    return _specularMaps;
}

const TexturePtr Material::normalMap() const { return _normalMap; }

const TexturePtr Material::depthMap() const { return _depthMap; }

float Material::shininess() const { return _shininess; }

float Material::heightScale() const { return _heightScale; }

bool Material::clipParallaxEdges() const { return _clipParallaxEdges; }

bool Material::hasNormalMap() const { return _hasNormalMap; }
