#include "components/behaviours/fly_camera.h"

#include "controls/input_state.h"
#include "scene/entity.h"

#include <SDL_scancode.h>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>

namespace {
    // The old 0.002 units per millisecond.
    constexpr float MOVEMENT_PER_SECOND = 0.005f;
    const float MOUSE_SENSITIVITY = glm::radians(0.167f);
    const float ZOOM_PER_NOTCH = glm::radians(1.67f);
    // Just short of straight up or down, where yaw stops meaning anything.
    const float MAX_PITCH = glm::half_pi<float>() - 0.001f;
    const float MIN_FOV = glm::radians(1.0f);
    const float MAX_FOV = glm::radians(120.0f);

    // -1, 0 or +1 for one axis; opposite keys cancel out.
    float axis(bool positive, bool negative) {
        return static_cast<float>(positive) - static_cast<float>(negative);
    }
}  // namespace

void FlyCamera::update(const FrameContext& context) {
    const InputState& input = context.input;
    Transform& transform = entity->getTransform();
    const glm::vec3 up(0.0f, 1.0f, 0.0f);

    // Look. Yaw turns around world up and pitch tilts around the camera's
    // own right, which never introduces roll. The pitch delta is limited so
    // the camera can't pass straight up or down.
    const glm::vec2 mouse = input.mouseDelta();
    const glm::vec3 lookDir = transform.getRot() * glm::vec3(0.0f, 0.0f, -1.0f);
    const float pitch = std::asin(glm::clamp(lookDir.y, -1.0f, 1.0f));
    const float pitchDelta = glm::clamp(-mouse.y * MOUSE_SENSITIVITY,
                                        -MAX_PITCH - pitch, MAX_PITCH - pitch);

    transform.rotate(up, -mouse.x * MOUSE_SENSITIVITY, Space::World);
    transform.rotate(glm::vec3(1.0f, 0.0f, 0.0f), pitchDelta, Space::Local);

    // Move. The basis comes from the rotation just applied: forward flattened
    // onto the ground, so looking down doesn't make you sink, and right,
    // already horizontal since there's no roll.
    const glm::quat rot = transform.getRot();
    glm::vec3 forward = rot * glm::vec3(0.0f, 0.0f, -1.0f);
    forward.y = 0.0f;
    forward = glm::normalize(forward);
    const glm::vec3 right = rot * glm::vec3(1.0f, 0.0f, 0.0f);

    const glm::vec3 direction =
        right * axis(input.held(SDL_SCANCODE_D), input.held(SDL_SCANCODE_A)) +
        up * axis(input.held(SDL_SCANCODE_SPACE),
                  input.held(SDL_SCANCODE_LCTRL)) +
        forward * axis(input.held(SDL_SCANCODE_W), input.held(SDL_SCANCODE_S));

    if (direction != glm::vec3(0.0f)) {
        transform.translate(glm::normalize(direction) * MOVEMENT_PER_SECOND *
                                context.dt,
                            Space::World);
    }

    // Zoom. Same direction as before: scrolling up widens the view.
    if (Camera* camera = entity->getCamera()) {
        camera->fovY = glm::clamp(
            camera->fovY + input.scroll() * ZOOM_PER_NOTCH, MIN_FOV, MAX_FOV);
    }
}
