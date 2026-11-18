#pragma once

#include "drawables/basic_mesh.h"
#include "scene/layers.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

constexpr std::uint32_t DEFAULT = 0xFFFFFFFF;

struct Outline {
    glm::vec3 color = glm::vec3(0.0f);
    float width = 0;
};

struct MeshRenderer {
    std::vector<std::shared_ptr<const BasicMesh>> parts;
    bool castsShadows = true;
    std::optional<Outline> outline = {};
    std::uint32_t layer = layers::DEFAULT;
};
