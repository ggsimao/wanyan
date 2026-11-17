#include "components/behaviours/picker.h"

#include "components/mesh_renderer.h"
#include "controls/input_state.h"
#include "scene/entity_handle.h"
#include "scene/scene.h"

#include <glm/gtx/quaternion.hpp>
#include <limits>
#include <optional>

namespace {
    constexpr float MAX_DISTANCE = 3.0f;
}  // namespace

void Picker::update(const FrameContext& context) {
    if (Entity* picked = context.scene.get(_picked)) {
        if (context.input.pressed(SDL_SCANCODE_E)) {
            picked->getBehaviour<Pickable>()->drop(context.scene);
            _picked = EntityHandle{};
        }
        return;
    }
    const Transform& cameraTransform = entity->getTransform();
    glm::vec3 cameraPos = cameraTransform.getPos();
    glm::vec3 cameraDir =
        cameraTransform.getRot() * glm::vec3(0.0f, 0.0f, -1.0f);
    if (Entity* previous = context.scene.get(_nearest)) {
        previous->getMeshRenderer()->outline = _nearestsOutline;
    }
    _nearest = EntityHandle{};
    _nearestDist = std::numeric_limits<float>::max();
    auto testDist = [&](Entity& tested) {
        if (!tested.getMeshRenderer())
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
                if (Entity* previous = context.scene.get(_nearest)) {
                    if (MeshRenderer* mesh = previous->getMeshRenderer()) {
                        mesh->outline = _nearestsOutline;
                    }
                }
                _nearestDist = dist;
                _nearest = tested.getHandle();
                _nearestsOutline = tested.getMeshRenderer()->outline;
                tested.getMeshRenderer()->outline =
                    Outline{.color = {1.0f, 1.0f, 0}, .width = 1.1f};
            }
        }
    };
    context.scene.forEachActive(testDist);
    if (context.input.pressed(SDL_SCANCODE_E)) {
        if (Entity* nearest = context.scene.get(_nearest)) {
            nearest->getBehaviour<Pickable>()->pickUp(context.scene,
                                                      entity->getHandle());
            _picked = _nearest;
            nearest->getMeshRenderer()->outline = _nearestsOutline;
        }
    }
}
