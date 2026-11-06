#pragma once

#include "controls.h"
#include "app/platform.h"

class PlatformController : public Slot, public Controller<Platform> {
public:
    void onSignal(const SignalType& signal) override;
    void processSignals(Platform& platform) override;

private:
    void onKeyPressed(const SDL_Keysym& key);

    bool _mouseCaptured = true;
};
