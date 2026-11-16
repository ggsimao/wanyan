#pragma once

#include "components/behaviour_base.h"
#include "components/frame_context.h"

#include <glm/ext/vector_float3.hpp>

struct Spin : BehaviourBase {
    glm::vec3 axis;
    float speed;  // radians per second
    void update(const FrameContext& context);
};
