#include "controls/controller_hub.h"

ControllerHub::ControllerHub() {
    _handler.connect(_program);
    _handler.connect(_renderOptions);
    _handler.connect(_screen);
    _handler.connect(_input);
    _handler.connect(_platform);
}

const InputState& ControllerHub::getInput() const { return *_input; }

void ControllerHub::update(const ControlTargets& targets) {
    // Before polling: key presses copy the current speed when they arrive.
    _input->beginFrame();
    _handler.waitEvent();

    _program->processSignals(targets.program);
    _renderOptions->processSignals(targets.renderOptions);
    _screen->processSignals(targets.screen);
    _platform->processSignals(targets.platform);
}
