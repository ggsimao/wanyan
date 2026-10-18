#include "controls/flashlight_controller.h"

void FlashlightController::onKeyPressed(const SDL_Keysym& key) {
    switch (key.sym) {
        case SDLK_f:
            _on = !_on;
            break;
        default:
            break;
    }
}

void FlashlightController::onSignal(const SignalType& signal) {
    if (const auto* pressed = std::get_if<SignalKeyPressed>(&signal)) {
        onKeyPressed(pressed->code);
    }
}

void FlashlightController::processSignals(Spotlight& spotlight) {
    spotlight.on = _on;
}
