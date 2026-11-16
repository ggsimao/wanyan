#pragma once

#include "drawables/basic_mesh.h"

#include <memory>
#include <optional>
#include <vector>

struct Outline {
    glm::vec3 color = glm::vec3(0.0f);
    float width = 0;
};

struct MeshRenderer {
    std::vector<std::shared_ptr<const BasicMesh>> parts;
    bool castsShadows = true;
    std::optional<Outline> outline = {};
};
