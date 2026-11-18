#include "components/behaviours/picker.h"

#include "components/mesh_renderer.h"
#include "controls/input_state.h"
#include "scene/entity_handle.h"
#include "scene/scene.h"

#include <glm/gtx/quaternion.hpp>
#include <limits>
#include <optional>
#include <utility>

namespace {
    constexpr float MAX_DISTANCE = 3.0f;
}  // namespace

void Picker::update(const FrameContext& context) {
    if (context.scene.get(_picked)) {
        if (context.input.pressed(SDL_SCANCODE_E)) {
            dropPicked(context);
        }
        return;
    }
    if (!entity->isAncestorOf(context.scene.getActiveCamera()->getHandle())) {
        return;
    }
    restoreOutline(context);
    const Transform& cameraTransform = entity->getTransform();
    glm::vec3 cameraPos = cameraTransform.getPos();
    glm::vec3 cameraDir =
        cameraTransform.getRot() * glm::vec3(0.0f, 0.0f, -1.0f);
    const auto applyOutline = [&](Entity& entity) {
        if (MeshRenderer* renderer = entity.getMeshRenderer()) {
            _savedOutlines.push_back(
                std::make_pair(entity.getHandle(), renderer->outline));
            renderer->outline =
                Outline{.color = {1.0f, 1.0f, 0}, .width = 1.1f};
        }
    };
    _nearest = EntityHandle{};
    _nearestDist = std::numeric_limits<float>::max();
    auto testDist = [&](Entity& tested) {
        if (&tested == entity || entity->isAncestorOf(tested.getHandle()) ||
            tested.isAncestorOf(entity->getHandle()))
            return;
        Pickable* behaviour = tested.getBehaviour<Pickable>();
        if (!behaviour)
            return;
        glm::vec3 center2Eye = tested.getTransform().getPos() - cameraPos;
        float dist = glm::dot(center2Eye, cameraDir);
        if (dist < 0 || dist > MAX_DISTANCE)
            return;
        float perpSqr = glm::length2(center2Eye) - dist * dist;
        if (perpSqr <= behaviour->radius * behaviour->radius) {
            if (dist < _nearestDist) {
                _nearestDist = dist;
                _nearest = tested.getHandle();
            }
        }
    };
    context.scene.forEachActive(testDist);
    if (auto nearest = context.scene.get(_nearest)) {
        context.scene.forEachInSubtree(*nearest, applyOutline);
    }
    if (context.input.pressed(SDL_SCANCODE_E)) {
        if (Entity* nearest = context.scene.get(_nearest)) {
            nearest->getBehaviour<Pickable>()->pickUp(context.scene,
                                                      entity->getHandle());
            _picked = _nearest;
            restoreOutline(context);
        }
    }
}

void Picker::dropPicked(const FrameContext& context) {
    if (Entity* picked = context.scene.get(_picked)) {
        picked->getBehaviour<Pickable>()->drop(context.scene);
        _picked = EntityHandle{};
    }
    restoreOutline(context);
}

void Picker::restoreOutline(const FrameContext& context) {
    for (auto& [handle, outline] : _savedOutlines) {
        if (Entity* e = context.scene.get(handle))
            if (MeshRenderer* r = e->getMeshRenderer())
                r->outline = outline;
    }
    _savedOutlines.clear();
}
