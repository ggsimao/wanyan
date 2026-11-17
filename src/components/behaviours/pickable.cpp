#include "components/behaviours/pickable.h"

#include "scene/entity.h"
#include "scene/entity_handle.h"
#include "scene/scene.h"

Pickable::Pickable(float radius) : radius(radius) {}

void Pickable::pickUp(Scene& scene, EntityHandle holder) {
    if (_picked)
        return;
    _picked = true;
    if (const Entity* parent = entity->getParent()) {
        _oldParent = parent->getHandle();
    } else {
        _oldParent = EntityHandle{};
    }
    scene.setParent(entity->getHandle(), holder, true);
    for (auto& b : *entity->getBehaviours()) {
        auto& base_b = base(b);
        if (&base_b == this)
            continue;
        _previousBehaviourStates.emplace(&base_b, base_b.active);
        base(b).active = false;
    }
}

void Pickable::drop(Scene& scene) {
    _picked = false;
    scene.setParent(entity->getHandle(), _oldParent, true);
    for (auto [b, a] : _previousBehaviourStates) {
        b->active = a;
    }
    _previousBehaviourStates.clear();
}
