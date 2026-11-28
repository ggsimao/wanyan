#include "controls/view_controller.h"

#include <algorithm>

void ViewController::onKeyPressed(const SDL_Keysym& key) {
    switch (key.sym) {
        case SDLK_x:
            _filterOn = !_filterOn;
            break;
        case SDLK_r:
            _mirror = !_mirror;
            break;
        case SDLK_m:
            _msaaOn = !_msaaOn;
            break;
        case SDLK_EQUALS:
            _gamma = std::min(_gamma + 0.2f, 3.0f);
            break;
        case SDLK_MINUS:
            _gamma = std::max(_gamma - 0.2f, 1.0f);
            break;
        default:
            break;
    }
}

void ViewController::onSignal(const SignalType& signal) {
    if (const auto* pressed = std::get_if<SignalKeyPressed>(&signal)) {
        onKeyPressed(pressed->code);
    }
}

void ViewController::processSignals(ViewParameters& parameters) {
    parameters.filterOn = _filterOn;
    parameters.msaaOn = _msaaOn;
    parameters.mirror = _mirror;
    parameters.gamma = _gamma;
}
