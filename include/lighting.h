#pragma once

#include "data/shadow_frame_buffer.h"

#include <GL/glew.h>
#include <glm/vec3.hpp>

#include <vector>

inline constexpr GLsizei SHADOW_RESOLUTION = 1024;

// The three Phong colour terms shared by every light, plus an on/off switch.
struct LightColors {
    glm::vec3 ambient{0.0f};
    glm::vec3 diffuse{0.0f};
    glm::vec3 specular{0.0f};
    bool on = true;

    // What the shader should receive: black while the light is switched off.
    glm::vec3 effectiveAmbient() const { return on ? ambient : glm::vec3(0.0f); }
    glm::vec3 effectiveDiffuse() const { return on ? diffuse : glm::vec3(0.0f); }
    glm::vec3 effectiveSpecular() const { return on ? specular : glm::vec3(0.0f); }
};

// Intensity falls off as 1 / (constant + linear * d + quadratic * d^2).
struct Attenuation {
    float constant = 1.0f;
    float linear = 0.0f;
    float quadratic = 0.0f;
};

// Infinitely far away, lighting everything from one direction (the sun).
// Owns the shadow map rendered from its point of view.
struct DirectionalLight : LightColors {
    DirectionalLight(const glm::vec3& lightDirection, const LightColors& colors);

    glm::vec3 direction;
    ShadowFrameBuffer shadowMap;
};

// Shines in all directions from a position, fading with distance.
struct PointLight : LightColors {
    PointLight(const glm::vec3& lightPosition, const LightColors& colors,
               const Attenuation& lightAttenuation);

    glm::vec3 position;
    Attenuation attenuation;
};

// A cone of light (a flashlight). Full intensity inside innerCutoff, fading to
// nothing at outerCutoff; both are half-angles in radians. Owns its shadow map.
struct Spotlight : LightColors {
    Spotlight(const glm::vec3& lightPosition, const glm::vec3& lightDirection,
              const LightColors& colors, const Attenuation& lightAttenuation,
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
};
