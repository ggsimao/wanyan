#include "components/behaviours/spin.h"
#include "scene/entity.h"

void Spin::update(const FrameContext& context) {
    entity->getTransform().rotate(context.dt *
                                  glm::quat(1000.0, 1.0, 1.0, 1.0));
}
