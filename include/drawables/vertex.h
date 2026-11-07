#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <type_traits>
#include <variant>

// Any unit tangent perpendicular to `normal`, for triangles whose texture
// coordinates can't define one (no UVs, or all three the same). Crossing with
// the axis least aligned with the normal keeps the result far from zero.
// `normal` must be non-zero.
glm::vec4 fallbackTangent(const glm::vec3& normal);

// The tangent a Vertex stores, from a raw tangent and bitangent: unit length,
// with its handedness in w (+1 or -1), so the shader can rebuild the bitangent
// as cross(N, T) * w. Falls back to fallbackTangent(normal) when `tangent` is
// unusable (non-finite or near zero). `normal` must be non-zero.
glm::vec4 packTangent(const glm::vec3& normal, const glm::vec3& tangent,
                      const glm::vec3& bitangent);

struct Vertex {
    glm::vec3 pos{0.0f}, normal{0.0f};
    glm::vec2 texCoords{0.0f};
    glm::vec4 tangent{0.0f};

    Vertex() = default;
    explicit Vertex(const glm::vec3& p);
    void translate(const glm::vec3& offset);
    void rotate(float angle, const glm::vec3& axis);
};

using VertexMember = std::variant<glm::vec2 Vertex::*, glm::vec3 Vertex::*, glm::vec4 Vertex::*>;

// One vertex attribute read from Vertex: its shader location and the offset
// of the field it reads. Every Vertex field is a vec3, so the format is fixed.
struct VertexAttribute {
    GLuint location;
    VertexMember member;
};

inline constexpr std::array VERTEX_ATTRIBUTES {
    VertexAttribute{.location=0, .member = &Vertex::pos},
    VertexAttribute{.location=1, .member = &Vertex::normal},
    VertexAttribute{.location=2, .member = &Vertex::texCoords},
    VertexAttribute{.location=3, .member = &Vertex::tangent},
};

static_assert(std::is_trivially_copyable_v<Vertex>);
static_assert(sizeof(Vertex) == 12 * sizeof(float)); // catches GLM alignment flags
