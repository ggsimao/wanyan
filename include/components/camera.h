#pragma once

#include "scene/layers.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>

struct Camera {
    float fovY = glm::radians(60.0f);
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    bool enabled = true;
    std::uint32_t mask = layers::ALL;
};
