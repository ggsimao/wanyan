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

// Whether `tangent` has the form packTangent produces: finite, unit length,
// with w exactly +1 or -1.
bool isValidTangent(const glm::vec4& tangent);

struct Vertex {
    glm::vec3 pos{0.0f}, normal{0.0f};
    glm::vec2 texCoords{0.0f};
    glm::vec4 tangent{0.0f};

    Vertex() = default;
    explicit Vertex(const glm::vec3& p);
};

using VertexMember =
    std::variant<glm::vec2 Vertex::*, glm::vec3 Vertex::*, glm::vec4 Vertex::*>;

// One vertex attribute read from Vertex: its shader location and the field it
// reads. The field's type decides the attribute's format (floats, as many as
// the vector has components), and each attribute gets its own buffer.
struct VertexAttribute {
    GLuint location;
    VertexMember member;
};

inline constexpr std::size_t POS_LOCATION = 0, NORMAL_LOCATION = 1,
                             TEX_COORDS_LOCATION = 2, TANGENT_LOCATION = 3;

// Every attribute a Vertex has, at fixed locations shared by all shaders.
// Instance attributes take the locations right after these (see MeshBuffers),
// and the shaders hardcode those (aInstModel at 4, aInstNormal at 8), so a new
// attribute here means updating every vertex shader.
inline constexpr std::array VERTEX_ATTRIBUTES{
    VertexAttribute{.location = POS_LOCATION, .member = &Vertex::pos},
    VertexAttribute{.location = NORMAL_LOCATION, .member = &Vertex::normal},
    VertexAttribute{.location = TEX_COORDS_LOCATION,
                    .member = &Vertex::texCoords},
    VertexAttribute{.location = TANGENT_LOCATION, .member = &Vertex::tangent},
};
static_assert(VERTEX_ATTRIBUTES.size() == 4,
              "Vertex shaders hardcode the instance locations after these: "
              "update them too");

static_assert(std::is_trivially_copyable_v<Vertex>);
static_assert(sizeof(Vertex) ==
              12 * sizeof(float));  // catches GLM alignment flags
