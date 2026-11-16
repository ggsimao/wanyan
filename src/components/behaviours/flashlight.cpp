#include "components/behaviours/flashlight.h"

#include "controls/input_state.h"
#include "scene/entity.h"

void Flashlight::update(const FrameContext& context) {
    if (context.input.pressed(SDL_SCANCODE_F)) {
        entity->getLight()->on ^= true;
    }
}
