#pragma once

#include "data/point_shadow_frame_buffer.h"
#include "data/shadow_frame_buffer.h"

#include <GL/glew.h>
#include <glm/fwd.hpp>
#include <glm/gtc/color_space.hpp>
#include <glm/vec3.hpp>

#include <cstddef>
#include <optional>
#include <vector>

inline constexpr GLsizei SHADOW_RESOLUTION = 1024;

struct FixtureIndices {
    size_t objectID;
    size_t instanceID;
};

// Color and intensity for every light, plus an on/off switch, and indices to
// its potential fixture.
struct Light {
    glm::vec3 color{0.0f};
    float intensity;
    bool on = true;
    std::optional<FixtureIndices> fixtureIndices = std::nullopt;

    // What the shader should receive: black while the light is switched off.
    glm::vec3 effectiveRadiance() const {
        return on ? glm::convertSRGBToLinear(color) * intensity
                  : glm::vec3(0.0f);
    }
};

// Intensity falls off as 1 / (constant + linear * d + quadratic * d^2).
struct Attenuation {
    float constant = 1.0f;
    float linear = 0.0f;
    float quadratic = 0.0f;
};

// Infinitely far away, lighting everything from one direction (the sun).
// Owns the shadow map rendered from its point of view.
struct DirectionalLight : Light {
    DirectionalLight(const glm::vec3& lightDirection, const Light& colors);

    glm::vec3 direction;
    ShadowFrameBuffer shadowMap;
};

// Shines in all directions from a position, fading with distance.
struct PointLight : Light {
    PointLight(const glm::vec3& lightPosition, const Light& colors,
               const Attenuation& lightAttenuation);

    glm::vec3 position;
    Attenuation attenuation;
    PointShadowFrameBuffer shadowMap;
};

// A cone of light (a flashlight). Full intensity inside innerCutoff, fading to
// nothing at outerCutoff; both are half-angles in radians. Owns its shadow map.
struct Spotlight : Light {
    Spotlight(const glm::vec3& lightPosition, const glm::vec3& lightDirection,
              const Light& colors, Attenuation attenuation,
              float innerCutoffAngle, float outerCutoffAngle);

    glm::vec3 position;
    glm::vec3 direction;
    Attenuation attenuation;
    float innerCutoff;  // phi
    float outerCutoff;  // gamma
    ShadowFrameBuffer shadowMap;
};

struct Lighting {
    DirectionalLight directional;
    std::vector<PointLight> points;
    Spotlight spotlight;
    glm::vec3 ambientColor;

    glm::vec3 effectiveAmbient() const {
        return glm::convertSRGBToLinear(ambientColor);
    }
};
