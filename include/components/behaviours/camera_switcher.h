#pragma once

#include "components/behaviour_base.h"
#include "components/frame_context.h"
#include "scene/entity_handle.h"

#include <vector>

// F2 to change camera
struct CameraSwitcher : BehaviourBase {
    explicit CameraSwitcher(std::vector<EntityHandle> cameras,
                            bool active = true)
        : BehaviourBase(active), _cameras(cameras) {}
    void update(const FrameContext& context);

  private:
    std::size_t _active = 0;
    std::vector<EntityHandle> _cameras;
};
