#include "components/behaviours/camera_switcher.h"

#include "controls/input_state.h"
#include "scene/entity_handle.h"
#include "scene/scene.h"

#include <SDL_scancode.h>

void CameraSwitcher::update(const FrameContext& context) {
    if (_cameras.empty())
        return;
    if (context.input.pressed(SDL_SCANCODE_F2)) {
        for (std::size_t step = 1; step < _cameras.size(); ++step) {
            const std::size_t next = (_active + step) % _cameras.size();
            EntityHandle nextHandle = _cameras[next];
            if (auto new_cam = context.scene.get(nextHandle)) {
                if (new_cam->activeInHierarchy()) {
                    if (auto old_cam = context.scene.get(_cameras[_active])) {
                        if (auto picker = old_cam->getBehaviour<Picker>()) {
                            picker->dropPicked(context);
                        }
                    }
                    context.scene.setActiveCamera(nextHandle);
                    _active = next;
                    break;
                }
            }
        }
    }
}
