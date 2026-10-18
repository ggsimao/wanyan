#pragma once

#include "controls/controls.h"
#include "screen.h"

// Screen effects: E toggles Sobel edge detection, M toggles the shader's MSAA
// resolve, = and - raise and lower gamma in steps of 0.2 (between 1 and 3).
class ScreenController : public Slot, public Controller<ScreenParameters> {
public:
    void onSignal(const SignalType& signal) override;
    void processSignals(ScreenParameters& parameters) override;

private:
    void onKeyPressed(const SDL_Keysym& key);

    bool _sobelOn = false;
    bool _msaaOn = true;
    float _gamma = DEFAULT_GAMMA;
};
