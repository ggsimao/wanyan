#pragma once

#include "components/behaviour_base.h"
#include "components/frame_context.h"

#include <glm/ext/vector_float3.hpp>
#include <optional>

struct SkyLightCycle : BehaviourBase {
  public:
    SkyLightCycle(float duration, float maxIntensity,
                  bool controlsAmbient = true);
    void update(const FrameContext& context);

  private:
    void calculateLight(const FrameContext& context);
    float _duration, _maxIntensity;
    std::optional<glm::vec3> _ambient;
    bool _controlsAmbient;
};
