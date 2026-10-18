#pragma once

#include "controls/controls.h"
#include "camera.h"

// WASD moves, Space and Left Ctrl move up and down, the mouse looks around
// and the wheel zooms.
//
// Keyboard movement is scaled by the frame time, so speed doesn't depend on
// frame rate. Mouse look and zoom aren't: a mouse movement or a wheel notch
// is already a fixed amount, however long the frame took.
class CameraController: public Slot, public Controller<Camera> {
public:
    // Call every frame, before input is polled.
    void setSpeeds(float frameMilliseconds);
    void onSignal(const SignalType& signal) override;
    void processSignals(Camera& obj) override;
private:
    void onKeyPressed(const SDL_Keysym& key);
    void onKeyReleased(const SDL_Keysym& key);
    void setHeld(const SDL_Keysym& key, bool held);
    void onMouseMoved(int dx, int dy);
    void onMouseScrolled(int delta);

    float _transSpeed = 0.0f;
    // Which movement keys are held. Movement is computed from these every
    // frame, so releasing a key always stops it exactly.
    bool _right = false, _left = false, _up = false, _down = false;
    bool _backward = false, _forward = false;
    // Accumulated since the last processSignals, then applied and reset.
    glm::vec3 _deltaRot = glm::vec3(0.0f);
    float _deltaZoom = 0.0f;
};
