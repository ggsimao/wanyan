#pragma once

#include "components/behaviour_base.h"
#include "components/frame_context.h"

struct SeasonCycle : BehaviourBase {
  public:
    void update(const FrameContext& context);
};
