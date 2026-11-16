#pragma once

#include "components/behaviour_base.h"
#include "components/frame_context.h"

// WASD moves horizontally, Space and Left Ctrl move along world Y, the mouse
// looks around and the wheel zooms.
struct FlyCamera : BehaviourBase {
    void update(const FrameContext& context);
};
