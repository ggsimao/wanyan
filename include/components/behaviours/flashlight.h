#pragma once

#include "components/behaviour_base.h"
#include "components/frame_context.h"

// F to turn flashlight on/off
struct Flashlight : BehaviourBase {
    void update(const FrameContext& context);
};
