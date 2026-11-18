#pragma once

#include "math.h"

#include <cmath>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <type_traits>

// One instance's data, exactly as the instanced vertex attributes read it:
// uploaded byte for byte, so its layout must not change (see the asserts).
struct Instance {
    glm::mat4 model;
    glm::mat3 normal;  // cofactor of model's upper 3x3; consumers normalize

    explicit Instance(const glm::mat4& world)
        : model(world), normal(cofactorDetMatrix(world)) {}

    // Byte offsets for the instance attribute setup.
    static constexpr std::size_t modelOffset() {
        return offsetof(Instance, model);
    }
    static constexpr std::size_t normalOffset() {
        return offsetof(Instance, normal);
    }
};

static_assert(std::is_trivially_copyable_v<Instance>);
static_assert(std::is_standard_layout_v<Instance>);
static_assert(sizeof(Instance) == sizeof(glm::mat4) + sizeof(glm::mat3));
