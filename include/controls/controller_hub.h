#pragma once

#include "controls/controls.h"
#include "controls/input_state.h"
#include "controls/platform_controller.h"
#include "controls/program_controller.h"
#include "controls/render_options_controller.h"
#include "controls/view_controller.h"

#include <memory>

// Everything the controllers act on, owned elsewhere (e.g. main).
struct ControlTargets {
    Program& program;
    RenderOptions& renderOptions;
    ViewParameters& view;
    Platform& platform;
};

// Every controller, and the signal handler that feeds input to the ones that
// need it. Call update() once per frame: input is polled every frame, so
// movement scaled by frame time never skips a frame.
class ControllerHub {
  public:
    // Connects the input-driven controllers to the signal handler.
    ControllerHub();

    const InputState& getInput() const;
    void update(const ControlTargets& targets);

  private:
    SignalHandler _handler;
    // shared_ptr because the handler keeps weak_ptrs to its slots.
    std::shared_ptr<ProgramController> _program =
        std::make_shared<ProgramController>();
    std::shared_ptr<RenderOptionsController> _renderOptions =
        std::make_shared<RenderOptionsController>();
    std::shared_ptr<ViewController> _view = std::make_shared<ViewController>();
    std::shared_ptr<InputState> _input = std::make_shared<InputState>();
    std::shared_ptr<PlatformController> _platform =
        std::make_shared<PlatformController>();
};
