#include "components/behaviours/spin.h"
#include "scene/entity.h"

void Spin::update(const FrameContext& context) {
    entity->getTransform().rotate({1.0f, 1.0f, 1.0f}, 0.0005 * context.dt);
}
