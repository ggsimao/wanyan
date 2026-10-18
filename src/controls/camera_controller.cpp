#include "controls/camera_controller.h"

namespace {
    // World units moved per millisecond while a movement key is held.
    constexpr float MOVEMENT_PER_MS = 0.002f;
    // Rotation per pixel of mouse movement, and field of view per wheel
    // notch, in radians (what Camera::rotate and changeFOV take). The values
    // Rust's frame-time scaling gave at 60 fps (16.67 ms frames): 0.167 and
    // 1.67 degrees. So the camera feels the same with vsync on.
    const float MOUSE_SENSITIVITY = glm::radians(0.167f);
    const float ZOOM_PER_NOTCH = glm::radians(1.67f);

    // -1, 0 or +1 for one axis; opposite keys cancel out.
    float axis(bool positive, bool negative) {
        return static_cast<float>(positive) - static_cast<float>(negative);
    }
}  // namespace

void CameraController::setSpeeds(float frameMilliseconds) {
    _transSpeed = frameMilliseconds * MOVEMENT_PER_MS;
}

void CameraController::onKeyPressed(const SDL_Keysym& key) {
    setHeld(key, true);
}

void CameraController::onKeyReleased(const SDL_Keysym& key) {
    setHeld(key, false);
}

void CameraController::setHeld(const SDL_Keysym& key, bool held) {
    switch (key.sym) {
        case SDLK_d:
            _right = held;
            break;
        case SDLK_a:
            _left = held;
            break;
        case SDLK_SPACE:
            _up = held;
            break;
        case SDLK_LCTRL:
            _down = held;
            break;
        case SDLK_s:
            _backward = held;
            break;
        case SDLK_w:
            _forward = held;
            break;
        default:
            break;
    }
}

void CameraController::onMouseMoved(int dx, int dy) {
    // Euler angles: x is pitch (moving the mouse up looks up), y is yaw.
    _deltaRot += glm::vec3(-dy, dx, 0.0f) * MOUSE_SENSITIVITY;
}

void CameraController::onMouseScrolled(int delta) {
    _deltaZoom += delta * ZOOM_PER_NOTCH;
}

void CameraController::onSignal(const SignalType& signal) {
    if (auto* skp = std::get_if<SignalKeyPressed>(&signal)) {
        onKeyPressed(skp->code);
    } else if (auto* skr = std::get_if<SignalKeyReleased>(&signal)) {
        onKeyReleased(skr->code);
    } else if (auto* smm = std::get_if<SignalMouseMoved>(&signal)) {
        onMouseMoved(smm->dx, smm->dy);
    } else if (auto* sms = std::get_if<SignalMouseScrolled>(&signal)) {
        onMouseScrolled(sms->delta);
    }
}

void CameraController::processSignals(Camera& obj) {
    const glm::vec3 movement =
        glm::vec3(axis(_right, _left), axis(_up, _down), axis(_backward, _forward)) * _transSpeed;

    obj.translateLongitudinal(movement.x);
    obj.translateVertical(movement.y);
    obj.translateForward(movement.z);
    obj.rotate(_deltaRot);
    obj.changeFOV(_deltaZoom);
    _deltaRot = glm::vec3(0.0f);
    _deltaZoom = 0.0f;
}
