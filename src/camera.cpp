
#include <camera.h>

#define GLM_ENABLE_EXPERIMENTAL

Camera::Camera(glm::vec3 init_pos, bool invert) : _inverted(invert), _pos(init_pos) {
    if (init_pos == glm::vec3(0.0)) {
        _dir = glm::vec3(0.0, 0.0, 1.0);
    } else {
        _dir = normalize(-init_pos);
    }
    // The inverse of rotate()'s direction formula. (Measuring angles to the
    // x axis with glm::angle failed for directions in the y-z plane, where
    // the pitch vector is zero, and lost the sign of the yaw.)
    _yaw = std::atan2(_dir.z, _dir.x);
    _pitch = std::asin(glm::clamp(_dir.y, -1.0f, 1.0f));
    _roll = 0.0;
    _fov = 1.0;
}

void Camera::translate(const glm::vec3& offset) {
    float sign = _inverted ? -1.0 : 1.0;

    glm::vec3 global_up = glm::vec3(0.0, 1.0, 0.0);
    glm::vec3 camera_right = normalize(cross(global_up, _dir));
    glm::vec3 camera_up = normalize(cross(_dir, camera_right));

    _pos -= sign * offset.z * _dir; // camera front
    _pos -= sign * offset.x * camera_right; // camera sideways
    _pos -= offset.y * camera_up; // camera vertical
}

void Camera::translateLongitudinal(float offset) {
    float sign = _inverted ? -1.0 : 1.0;

    glm::vec3 global_up = glm::vec3(0.0, 1.0, 0.0);
    glm::vec3 camera_right = normalize(cross(global_up, _dir));

    _pos -= sign * offset * camera_right;
}

void Camera::translateAxial(float offset) {
    glm::vec3 global_up = glm::vec3(0.0, 1.0, 0.0);
    glm::vec3 camera_right = normalize(cross(global_up, _dir));
    glm::vec3 camera_up = cross(_dir, camera_right);

    _pos -= offset * camera_up; // camera vertical
}

void Camera::translateFrontal(float offset) {
    float sign = _inverted ? -1.0 : 1.0;
    _pos -= sign * offset * _dir; // camera front
}

void Camera::translateForward(float offset) {
    glm::vec3 direction = glm::vec3(0.0);
    direction.x = cosf(_yaw);
    direction.z = sinf(_yaw);
    direction *= _inverted ? -offset : offset;
    _pos -= direction;
}

void Camera::translateVertical(float offset) {
    _pos.y += offset;
}

void Camera::rotate(const glm::vec3& euler_angles) {
    _pitch = std::min(
        std::max(
            _pitch + euler_angles.x,
            -glm::pi<float>() / 2.0f + ANGLE_LOWER_BOUND
        ),
        glm::pi<float>() / 2.0f - ANGLE_LOWER_BOUND
    );
    _yaw += euler_angles.y;
    _roll += euler_angles.z;

    _dir.x = cos(_yaw) * cos(_pitch);
    _dir.y = sin(_pitch);
    _dir.z = sin(_yaw) * cos(_pitch);
}

void Camera::rotatePitch(float rotation) {
    _pitch = std::min(
        std::max(
            _pitch + rotation,
            -glm::pi<float>() / 2.0f + ANGLE_LOWER_BOUND
        ),
        glm::pi<float>() / 2.0f - ANGLE_LOWER_BOUND
    );

    _dir.x = cos(_yaw) * cos(_pitch);
    _dir.y = sin(_pitch);
    _dir.z = sin(_yaw) * cos(_pitch);
}

void Camera::rotateYaw(float rotation) {
    _yaw += rotation;

    _dir.x = cos(_yaw) * cos(_pitch);
    _dir.z = sin(_yaw) * cos(_pitch);
}

void Camera::rotateRoll(float rotation) {
    _roll += rotation;
}

void Camera::changeFOV(float offset) {
    _fov += offset;
}

float Camera::pitch() const {
    return _pitch;
}

float Camera::yaw() const {
    return _yaw;
}

float Camera::roll() const {
    return _roll;
}

float Camera::fov() const {
    return _fov;
}

glm::vec3 Camera::pos() const {
    return _pos;
}

glm::vec3 Camera::dir() const {
    return _dir;
}

glm::mat4 Camera::lookAt() const {
    return glm::lookAt(_pos, (_dir + _pos), glm::vec3(0.0, 1.0, 0.0));
}

// The mirror's view: the same position, looking back the opposite way and
// with the pitch flipped, as in Rust. Angles here are in radians.
Camera Camera::invert() const {
    Camera inverted = *this;
    inverted.rotatePitch(-2.0f * _pitch);
    inverted.rotateYaw(glm::pi<float>());
    return inverted;
}
