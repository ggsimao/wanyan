#include "controls/platform_controller.h"
#include "controls/controls.h"
#include <SDL_keycode.h>
#include <variant>

void PlatformController::onSignal(const SignalType& signal) {
    if (auto* s = std::get_if<SignalKeyPressed>(&signal)) {
        onKeyPressed(s->code);
    } else if (std::holds_alternative<SignalFocusLost>(signal)) {
        _mouseCaptured = false;
    } else if (std::holds_alternative<SignalCaptureRequested>(signal)) {
        _mouseCaptured = true;
    }
}

void PlatformController::processSignals(Platform& platform) {
    platform.setMouseCapture(_mouseCaptured);
}

void PlatformController::onKeyPressed(const SDL_Keysym& key) {
    switch (key.sym) {
        case SDLK_F1:
            _mouseCaptured = !_mouseCaptured;
            break;
        default:
            break;
    }
}
