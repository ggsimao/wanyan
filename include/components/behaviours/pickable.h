#pragma once

#include "components/behaviour_base.h"
#include "components/frame_context.h"
#include "scene/entity_handle.h"

#include <map>

// E to pick up nearest object in front of the camera
struct Pickable : BehaviourBase {
    float radius;
    explicit Pickable(float radius = 0.5f, bool active = true)
        : BehaviourBase(active), radius(radius) {}
    void pickUp(Scene& scene, EntityHandle holder);
    void drop(Scene& scene);

  private:
    bool _picked = false;
    EntityHandle _oldParent;
    std::map<BehaviourBase*, bool> _previousBehaviourStates;
};
