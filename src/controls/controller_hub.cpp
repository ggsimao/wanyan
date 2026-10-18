#include "controls/controller_hub.h"

ControllerHub::ControllerHub() {
    _handler.connect(_program);
    _handler.connect(_camera);
    _handler.connect(_flashlight);
    _handler.connect(_renderOptions);
    _handler.connect(_screen);
}

void ControllerHub::update(float frameMilliseconds, const ControlTargets& targets) {
    // Before polling: key presses copy the current speed when they arrive.
    _camera->setSpeeds(frameMilliseconds);
    _handler.waitEvent();

    _program->processSignals(targets.program);
    _camera->processSignals(targets.camera);
    _flashlight->processSignals(targets.flashlight);
    _renderOptions->processSignals(targets.renderOptions);
    _screen->processSignals(targets.screen);
    _randomTransforms.processSignals(targets.rockTransforms);
}
