#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/ext/matrix_transform.hpp>

inline const float ANGLE_LOWER_BOUND = 0.001;

class Camera {
public:
    Camera(glm::vec3 init_pos, bool invert = false);
    void translate(const glm::vec3& offset);
    void translateLongitudinal(float offset);
    void translateAxial(float offset);
    void translateFrontal(float offset);
    void translateForward(float offset);
    void translateVertical(float offset);
    void rotate(const glm::vec3& euler_angles);
    void rotatePitch(float rotation);
    void rotateYaw(float rotation);
    void rotateRoll(float rotation);
    void changeFOV(float offset);
    float pitch() const;
    float yaw() const;
    float roll() const;
    float fov() const;
    glm::vec3 pos() const;
    glm::vec3 dir() const;
    glm::mat4 lookAt() const;
    Camera invert() const;

    friend class CameraController;
private:
    bool _inverted;
    glm::vec3 _pos = glm::vec3(0.0), _dir = glm::vec3(0.0);
    float _pitch = 0.0, _yaw = 0.0, _roll = 0.0, _fov = 1.0;
};
