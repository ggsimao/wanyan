#include "random_transform.h"

#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <random>

namespace {
    std::mt19937& engine() {
        static std::mt19937 instance{std::random_device{}()};
        return instance;
    }

    float randomIn(Range range) {
        return std::uniform_real_distribution<float>(range.min, range.max)(engine());
    }

    // A direction picked uniformly over the sphere. Rejection sampling: points in
    // the cube outside the unit ball, or too close to the centre to normalize
    // safely, are retried. (Normalizing any point of the cube, as the Rust
    // version did, favours the cube's corners and can divide by zero.)
    glm::vec3 randomUnitVector() {
        std::uniform_real_distribution<float> coordinate(-1.0f, 1.0f);
        while (true) {
            const glm::vec3 v(coordinate(engine()), coordinate(engine()), coordinate(engine()));
            const float lengthSquared = glm::dot(v, v);
            if (lengthSquared > 1e-6f && lengthSquared <= 1.0f) {
                return v / std::sqrt(lengthSquared);
            }
        }
    }
}  // namespace

RandomTransform::RandomTransform(float angularStep, float linearStep, unsigned angularRate,
                                 unsigned linearRate)
    : _angularStep(angularStep),
      _linearStep(linearStep),
      _axisCountdown{angularRate, angularRate},
      _directionCountdown{linearRate, linearRate} {
    randomizeAxis();
    randomizeDirection();
}

void RandomTransform::tick() {
    if (_axisCountdown.tick()) {
        randomizeAxis();
    }
    if (_directionCountdown.tick()) {
        randomizeDirection();
    }
}

void RandomTransform::randomizeAxis() {
    _axis = randomUnitVector();
    _rotation = glm::rotate(glm::mat4(1.0f), _angularStep, _axis);
}

void RandomTransform::randomizeDirection() {
    _direction = randomUnitVector();
    _translation = _linearStep * _direction;
}

glm::vec3 RandomTransform::randomOffset(Range x, Range y, Range z) {
    return {randomIn(x), randomIn(y), randomIn(z)};
}

bool RandomTransform::Countdown::tick() {
    if (rate == 0) {
        return false;
    }
    if (--remaining == 0) {
        remaining = rate;
        return true;
    }
    return false;
}
