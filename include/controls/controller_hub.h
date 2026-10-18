#pragma once

#include "controls/camera_controller.h"
#include "controls/controls.h"
#include "controls/flashlight_controller.h"
#include "controls/program_controller.h"
#include "controls/random_transform_controller.h"
#include "controls/render_options_controller.h"
#include "controls/screen_controller.h"

#include <memory>
#include <vector>

// Everything the controllers act on, owned elsewhere (e.g. main).
struct ControlTargets {
    Program& program;
    Camera& camera;
    Spotlight& flashlight;
    RenderOptions& renderOptions;
    ScreenParameters& screen;
    std::vector<RandomTransform>& rockTransforms;
};

// Every controller, and the signal handler that feeds input to the ones that
// need it. Call update() once per frame: input is polled every frame, so
// movement scaled by frame time never skips a frame.
class ControllerHub {
public:
    // Connects the input-driven controllers to the signal handler.
    ControllerHub();

    void update(float frameMilliseconds, const ControlTargets& targets);

private:
    SignalHandler _handler;
    // shared_ptr because the handler keeps weak_ptrs to its slots.
    std::shared_ptr<ProgramController> _program = std::make_shared<ProgramController>();
    std::shared_ptr<CameraController> _camera = std::make_shared<CameraController>();
    std::shared_ptr<FlashlightController> _flashlight = std::make_shared<FlashlightController>();
    std::shared_ptr<RenderOptionsController> _renderOptions =
        std::make_shared<RenderOptionsController>();
    std::shared_ptr<ScreenController> _screen = std::make_shared<ScreenController>();
    // Driven by the frame, not input, so it isn't connected to the handler.
    RandomTransformController _randomTransforms;
};
