#include "math.h"

#include "random.h"

#include <random>

glm::mat3 cofactorDetMatrix(const glm::mat4& model) {
    auto det = glm::determinant(model);
    auto cofactor = glm::mat3(model);
    if (det < 0) {
        cofactor *= -1;
    }
    return cofactor;
}

// A direction picked uniformly over the sphere. Rejection sampling: points
// in the cube outside the unit ball, or too close to the centre to
// normalize safely, are retried. (Normalizing any point of the cube, as the
// Rust version did, favours the cube's corners and can divide by zero.)
glm::vec3 randomUnitVector() {
    std::uniform_real_distribution<float> coordinate(-1.0f, 1.0f);
    while (true) {
        const glm::vec3 v(coordinate(random_engine::engine()),
                          coordinate(random_engine::engine()),
                          coordinate(random_engine::engine()));
        const float lengthSquared = glm::dot(v, v);
        if (lengthSquared > 1e-6f && lengthSquared <= 1.0f) {
            return v / std::sqrt(lengthSquared);
        }
    }
}

float randomIn(Range range) {
    return std::uniform_real_distribution<float>(range.min, range.max)(
        random_engine::engine());
}
