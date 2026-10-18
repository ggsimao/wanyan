#include "controls/render_options_controller.h"

void RenderOptionsController::onKeyPressed(const SDL_Keysym& key) {
    switch (key.sym) {
        case SDLK_n:
            _visualizeNormals = !_visualizeNormals;
            break;
        default:
            break;
    }
}

void RenderOptionsController::onSignal(const SignalType& signal) {
    if (const auto* pressed = std::get_if<SignalKeyPressed>(&signal)) {
        onKeyPressed(pressed->code);
    }
}

void RenderOptionsController::processSignals(RenderOptions& options) {
    options.visualizeNormals = _visualizeNormals;
}
