#include "components/behaviours/sky_light_cycle.h"

#include "components/frame_context.h"
#include "scene/scene.h"
#include <optional>

SkyLightCycle::SkyLightCycle(float duration, float maxIntensity,
                             bool controlsAmbient)
    : _duration(duration), _maxIntensity(maxIntensity),
      _ambient(glm::vec3(0.0f)), _controlsAmbient(controlsAmbient) {}

void SkyLightCycle::update(const FrameContext& context) {
    float days = context.scene.time.days;
    const float dayFraction =
        std::fmod(days, _duration) / _duration;  // 0 at midnight, 0.5 at noon
    const float hourAngle =
        glm::two_pi<float>() * (dayFraction - 0.5f);  // 0 at solar noon
    entity->getTransform().setRotation(
        glm::angleAxis(-hourAngle, glm::vec3(0, 1, 0)), Space::Local);
    calculateLight(context);
}

void SkyLightCycle::calculateLight(const FrameContext& context) {
    if (auto light = entity->getLight()) {
        const glm::vec3 direction = glm::normalize(
            entity->getTransform().getRot() * glm::vec3(0.0f, 0.0f, -1.0f));
        const float elevation = -direction.y;
        const float visible = glm::smoothstep(
            -0.05f, 0.1f, elevation);  // 0 below the horizon, 1 a bit above it
        float intensity = _maxIntensity * visible;
        if (!_ambient && _controlsAmbient) {
            _ambient = context.scene.ambient;
        }
        if (_ambient && _controlsAmbient) {
            context.scene.ambient = _ambient.value() * visible;
        }
        if (intensity <= 0) {
            light->on = false;
            return;
        }
        light->on = true;
        const float zenithDeg = 90.0f - glm::degrees(std::asin(elevation));
        const float airMass =
            1.0f /
            (elevation + 0.50572f * std::pow(96.07995f - zenithDeg, -1.6364f));
        const glm::vec3 scatter =
            0.1f * glm::vec3(0.43f, 1.0f, 2.45f);  // (550 nm / λ)^4 for R, G, B
        const glm::vec3 transmittance = glm::exp(-scatter * airMass);
        const glm::vec3 color =
            transmittance / glm::exp(-scatter);  // normalised so noon is white
        light->color = color;
        light->intensity = intensity;
    }
}
