#include "program.h"

Program::Program() : _loopActive(true) {}

bool Program::loopActive() const {
    return _loopActive;
}

bool Program::showStats() const {
    return _showStats;
}
