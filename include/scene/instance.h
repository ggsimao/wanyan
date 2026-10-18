#pragma once

#include "spatial.h"

#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <cstddef>
#include <type_traits>

// One copy of a SceneObject: its model matrix and the matching normal matrix.
// Uploaded to the GPU byte for byte as per-instance vertex attributes, so its
// layout is fixed: the model matrix at locations 3-6 (one vec4 column each),
// the normal matrix at 7-9 (one vec3 column each).
class Instance : public Spatial<Instance> {
public:
    const glm::mat4& model() const { return _model; }
    // Keeps the normal matrix in step with the model matrix, since both are
    // uploaded together.
    void setModel(const glm::mat4& model) {
        _model = model;
        _normal = normalMatrix();
    }
    const glm::mat3& normal() const { return _normal; }

    // Byte offsets for the instance attribute setup.
    static constexpr std::size_t modelOffset() { return offsetof(Instance, _model); }
    static constexpr std::size_t normalOffset() { return offsetof(Instance, _normal); }

private:
    glm::mat4 _model{1.0f};
    glm::mat3 _normal{1.0f};
};

static_assert(std::is_trivially_copyable_v<Instance>,
              "Instance is uploaded with glBufferData and must be plain data");
static_assert(std::is_standard_layout_v<Instance>, "offsetof needs a standard-layout type");
static_assert(sizeof(Instance) == sizeof(glm::mat4) + sizeof(glm::mat3),
              "No padding or hidden members: the GPU reads Instance's bytes directly");
