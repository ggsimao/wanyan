#pragma once

#include "components/behaviour_base.h"
#include "components/frame_context.h"

// F to turn flashlight on/off
struct Flashlight : BehaviourBase {
    explicit Flashlight(bool active = true) : BehaviourBase(active) {}
    void update(const FrameContext& context);
};
