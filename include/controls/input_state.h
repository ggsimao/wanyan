#pragma once

#include "controls/controls.h"
#include <bitset>

class InputState : public Slot {
  public:
    void beginFrame();  // clears pressed/released/mouse delta, before polling
    void onSignal(const SignalType& signal) override;

    bool held(SDL_Keycode key) const;     // down right now
    bool pressed(SDL_Keycode key) const;  // went down this frame (toggles)
    bool released(SDL_Keycode key) const;
    glm::vec2 mouseDelta() const;
    int scroll() const;

  private:
    std::bitset<SDL_NUM_SCANCODES> _held, _pressed, _released;
    glm::vec2 _mouseDelta;
    int _scroll;
};
