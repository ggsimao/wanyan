#pragma once

#include "spatial.h"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

// An inclusive range for random values.
struct Range {
    float min;
    float max;
};

// A random rotation and drift applied to an object every frame. The axis and
// direction are re-randomized periodically, every `angularRate` and
// `linearRate` ticks (0 means never).
class RandomTransform {
public:
    RandomTransform(float angularStep, float linearStep, unsigned angularRate,
                    unsigned linearRate);

    // Moves an object by a random offset within the given ranges.
    template <typename T>
    static void position(Spatial<T>& object, Range x, Range y, Range z) {
        object.translate(randomOffset(x, y, z));
    }

    template <typename T>
    void rotate(Spatial<T>& object) const {
        object.applyRotation(_rotation);
    }

    template <typename T>
    void translate(Spatial<T>& object) const {
        object.translate(_translation);
    }

    // Advances the countdowns, re-randomizing the axis or direction when
    // theirs run out. Call once per frame.
    void tick();

    void randomizeAxis();
    void randomizeDirection();

private:
    // A countdown that fires every `rate` ticks; rate 0 never fires.
    struct Countdown {
        unsigned rate;
        unsigned remaining;
        bool tick();
    };

    static glm::vec3 randomOffset(Range x, Range y, Range z);

    float _angularStep;
    float _linearStep;
    Countdown _axisCountdown;
    Countdown _directionCountdown;
    glm::vec3 _axis{0.0f, 1.0f, 0.0f};
    glm::vec3 _direction{1.0f, 0.0f, 0.0f};
    glm::mat4 _rotation{1.0f};
    glm::vec3 _translation{0.0f};
};
