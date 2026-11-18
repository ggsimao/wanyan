#include "components/behaviours/flashlight.h"

#include "controls/input_state.h"
#include "scene/entity.h"
#include "scene/scene.h"

void Flashlight::update(const FrameContext& context) {
    if (const auto camera = context.scene.getActiveCamera();
        camera && !entity->isAncestorOf(camera->getHandle()))
        return;
    if (context.input.pressed(SDL_SCANCODE_F)) {
        entity->getLight()->on ^= true;
    }
}
