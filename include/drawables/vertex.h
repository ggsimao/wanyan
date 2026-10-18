#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <type_traits>

struct Vertex {
    glm::vec3 pos{0.0f}, normal{0.0f}, texCoords{0.0f};

    Vertex() = default;
    explicit Vertex(const glm::vec3& p);
    void translate(const glm::vec3& offset);
    void rotate(float angle, const glm::vec3& axis);
};
static_assert(std::is_trivially_copyable_v<Vertex>);
static_assert(sizeof(Vertex) == 9 * sizeof(float)); // catches GLM alignment flags
