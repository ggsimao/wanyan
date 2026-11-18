#pragma once

#include "components/behaviour_base.h"
#include "components/frame_context.h"
#include "components/mesh_renderer.h"
#include "scene/entity_handle.h"

#include <limits>
#include <optional>

// E to pick up nearest object in front of the camera
struct Picker : BehaviourBase {
    explicit Picker(bool active = true) : BehaviourBase(active) {}
    void update(const FrameContext& context);
    void dropPicked(const FrameContext& context);

  private:
    void restoreOutline(const FrameContext& context);

    EntityHandle _nearest, _picked;
    float _nearestDist = std::numeric_limits<float>::max();
    std::vector<std::pair<EntityHandle, std::optional<Outline>>> _savedOutlines;
};
