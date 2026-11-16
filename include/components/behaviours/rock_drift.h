#pragma once

#include "components/behaviour_base.h"
#include "components/frame_context.h"

#include <glm/ext/vector_float3.hpp>

class RockDrift : public BehaviourBase {
  public:
    RockDrift(float angularStep, float linearStep, unsigned angularRate,
              unsigned linearRate);

    void start(const FrameContext& context);
    void update(const FrameContext& context);

  private:
    // A countdown that fires every `rate` frames; rate 0 never fires.
    struct Countdown {
        unsigned rate;
        unsigned remaining;
        bool tick();
    };

    void randomizeAxis();
    void randomizeDirection();

    float _angularStep;  // radians per frame
    float _linearStep;   // world units per frame
    glm::vec3 _axis;     // current spin axis, a unit vector
    glm::vec3 _direction;
    Countdown _axisCountdown;
    Countdown _directionCountdown;
};
