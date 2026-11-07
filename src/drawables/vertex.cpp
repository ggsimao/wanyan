#include "drawables/vertex.h"

namespace {
    // Below this, a tangent's direction is mostly rounding error.
    constexpr float MIN_TANGENT_LENGTH = 1e-6f;
}

glm::vec4 fallbackTangent(const glm::vec3& normal) {
    assert(glm::length(normal) > 0.0f && "fallbackTangent needs a non-zero normal");
    const glm::vec3 magnitude = glm::abs(normal);
    glm::vec3 axis(0.0f);
    if (magnitude.x <= magnitude.y && magnitude.x <= magnitude.z) {
        axis.x = 1.0f;
    } else if (magnitude.y <= magnitude.z) {
        axis.y = 1.0f;
    } else {
        axis.z = 1.0f;
    }
    return glm::vec4(glm::normalize(glm::cross(normal, axis)), 1.0f);
}

glm::vec4 packTangent(const glm::vec3& normal, const glm::vec3& tangent,
                      const glm::vec3& bitangent) {
    // A NaN or infinite component makes the length non-finite too, so this
    // one check covers both kinds of unusable tangent.
    const float length = glm::length(tangent);
    if (!std::isfinite(length) || length < MIN_TANGENT_LENGTH) {
        return fallbackTangent(normal);
    }
    const float w = glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f ? -1.0f : 1.0f;
    return glm::vec4(tangent / length, w);
}

bool isValidTangent(const glm::vec4& tangent) {
    const float length = glm::length(glm::vec3(tangent));
    return std::isfinite(length) && std::abs(length - 1.0f) < 1e-3f
        && (tangent.w == 1.0f || tangent.w == -1.0f);
}

Vertex::Vertex(const glm::vec3& pos) : pos(pos), normal(glm::vec3(0.0f)), texCoords(glm::vec3(0.0f)) {}
