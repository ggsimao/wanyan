#include "components/behaviours/season_cycle.h"

#include "scene/scene.h"

#include <glm/trigonometric.hpp>

void SeasonCycle::update(const FrameContext& context) {
    const float declination =
        -glm::radians(23.44f) *
        std::cos(glm::two_pi<float>() * context.scene.time.days /
                 context.scene.time.yearDuration);
    entity->getTransform().setRotation(
        glm::angleAxis(-declination, glm::vec3(1, 0, 0)), Space::Local);
}
