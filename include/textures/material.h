#pragma once

#include "textures/texture_2d.h"

#include <vector>

struct MaterialParameters {
    std::vector<TexturePtr> diff = {};
    std::vector<TexturePtr> spec = {};
    TexturePtr normalMap = nullptr;
    TexturePtr depthMap = nullptr;
    float shininess = 0.0f;
    float heightScale = 0.0f;
    bool clipParallaxEdges = false;
};

// Diffuse and specular maps plus shininess. Always holds at least one map of
// each kind: missing ones are filled with Texture2D::empty().
class Material {
  public:
    Material(MaterialParameters params);

    const std::vector<TexturePtr>& diffuseMaps() const;
    const std::vector<TexturePtr>& specularMaps() const;
    const TexturePtr normalMap() const;
    const TexturePtr depthMap() const;
    float shininess() const;
    float heightScale() const;
    bool clipParallaxEdges() const;
    bool hasNormalMap() const;

  private:
    std::vector<TexturePtr> _diffuseMaps, _specularMaps;
    TexturePtr _normalMap, _depthMap;
    float _shininess, _heightScale;
    bool _clipParallaxEdges, _hasNormalMap = false;
};
