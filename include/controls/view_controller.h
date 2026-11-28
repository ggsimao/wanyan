#pragma once

#include "controls/controls.h"
#include "view.h"

// View effects: E toggles edge filter, M toggles the shader's MSAA
// resolve, = and - raise and lower gamma in steps of 0.2 (between 1 and 3).
class ViewController : public Slot, public Controller<ViewParameters> {
  public:
    void onSignal(const SignalType& signal) override;
    void processSignals(ViewParameters& parameters) override;

  private:
    void onKeyPressed(const SDL_Keysym& key);

    bool _filterOn = false;
    bool _msaaOn = true;
    bool _mirror = false;
    float _gamma = DEFAULT_GAMMA;
};
