#include "components/behaviours/rock_drift.h"

#include "components/frame_context.h"
#include "math.h"
#include "random.h"
#include "scene/entity.h"

RockDrift::RockDrift(float angularStep, float linearStep, unsigned angularRate,
                     unsigned linearRate)
    : _angularStep(angularStep), _linearStep(linearStep),
      _axisCountdown(angularRate, angularRate),
      _directionCountdown(linearRate, linearRate) {
    randomizeAxis();
    randomizeDirection();
}

void RockDrift::start(const FrameContext& context) {
    random_engine::setSeed(context.seed);
}

void RockDrift::update(const FrameContext& context) {
    auto& transform = entity->getTransform();
    transform.rotate(_axis, _angularStep * 0.1f * context.dt);
    transform.translate(_linearStep * 0.1f * context.dt * _direction);
    if (_axisCountdown.tick()) {
        randomizeAxis();
    }
    if (_directionCountdown.tick()) {
        randomizeDirection();
    }
}

void RockDrift::randomizeAxis() { _axis = randomUnitVector(); }

void RockDrift::randomizeDirection() { _direction = randomUnitVector(); }

bool RockDrift::Countdown::tick() {
    if (rate == 0) {
        return false;
    }
    if (--remaining == 0) {
        remaining = rate;
        return true;
    }
    return false;
}
