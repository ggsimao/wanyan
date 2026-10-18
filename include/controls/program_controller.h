#pragma once

#include "controls/controls.h"
#include "program.h"

// Escape (or closing the window) quits; F3 toggles the frame statistics.
class ProgramController : public Slot, public Controller<Program> {
public:
    void onSignal(const SignalType& signal) override;
    void processSignals(Program& obj) override;
private:
    void onKeyPressed(const SDL_Keysym& key);
    bool _quit = false;
    bool _showStats = false;
};
