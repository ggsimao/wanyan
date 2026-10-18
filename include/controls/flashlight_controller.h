#pragma once

#include "controls.h"
#include "lighting.h"

// Toggles a spotlight with the F key.
class FlashlightController : public Slot, public Controller<Spotlight> {
public:
    void onSignal(const SignalType& signal) override;
    void processSignals(Spotlight& spotlight) override;

private:
    void onKeyPressed(const SDL_Keysym& key);

    // Starts off, as in the Rust version: the first frame switches the
    // flashlight off until F is pressed.
    bool _on = false;
};
