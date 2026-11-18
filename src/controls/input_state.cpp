#include "controls/input_state.h"

void InputState::beginFrame() {
    _pressed.reset();
    _released.reset();
    _mouseDelta = {
        0.0f,
        0.0f,
    };
    _scroll = 0;
}

void InputState::onSignal(const SignalType& signal) {
    if (auto* p = std::get_if<SignalKeyPressed>(&signal)) {
        _held[p->code.scancode] = true;
        _pressed[p->code.scancode] = true;
    } else if (auto* r = std::get_if<SignalKeyReleased>(&signal)) {
        _held[r->code.scancode] = false;
        _released[r->code.scancode] = true;
    } else if (auto* m = std::get_if<SignalMouseMoved>(&signal)) {
        _mouseDelta += glm::vec2(m->dx, m->dy);
    } else if (auto* s = std::get_if<SignalMouseScrolled>(&signal)) {
        _scroll += s->delta;
    } else if (std::holds_alternative<SignalFocusLost>(signal)) {
        _held.reset();
    }
}

bool InputState::held(SDL_Keycode key) const { return _held[key]; }

bool InputState::pressed(SDL_Keycode key) const { return _pressed[key]; }

bool InputState::released(SDL_Keycode key) const { return _released[key]; }

glm::vec2 InputState::mouseDelta() const { return _mouseDelta; }

int InputState::scroll() const { return _scroll; }
