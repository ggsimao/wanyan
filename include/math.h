#pragma once

#include <glm/gtc/matrix_transform.hpp>

// An inclusive range for random values.
struct Range {
    float min;
    float max;
};

glm::mat3 cofactorDetMatrix(const glm::mat4& model);

// A direction picked uniformly over the sphere. Rejection sampling: points
// in the cube outside the unit ball, or too close to the centre to
// normalize safely, are retried. (Normalizing any point of the cube, as the
// Rust version did, favours the cube's corners and can divide by zero.)
glm::vec3 randomUnitVector();

float randomIn(Range range);
