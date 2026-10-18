#include "controls/program_controller.h"

void ProgramController::onKeyPressed(const SDL_Keysym& key) {
    switch (key.sym) {
        case SDLK_ESCAPE:
            _quit = true;
            break;
        case SDLK_F3:
            _showStats = !_showStats;
            break;
        default:
            break;
    }
}

void ProgramController::onSignal(const SignalType& signal) {
    if (auto* s = std::get_if<SignalKeyPressed>(&signal)) {
        onKeyPressed(s->code);
    } else if (std::holds_alternative<SignalQuit>(signal)) {
        _quit = true;
    }
}

void ProgramController::processSignals(Program& obj) {
    obj._loopActive = !_quit;
    obj._showStats = _showStats;
}
