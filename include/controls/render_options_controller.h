#pragma once

#include "controls/controls.h"
#include "scene/scene_renderer.h"

// Toggles the renderer's normal visualization with the N key.
class RenderOptionsController : public Slot, public Controller<RenderOptions> {
public:
    void onSignal(const SignalType& signal) override;
    void processSignals(RenderOptions& options) override;

private:
    void onKeyPressed(const SDL_Keysym& key);

    bool _visualizeNormals = false;
};
