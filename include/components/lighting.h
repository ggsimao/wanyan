#pragma once

#include "scene/entity_handle.h"

#include <GL/glew.h>
#include <glm/fwd.hpp>
#include <glm/gtc/color_space.hpp>
#include <glm/vec3.hpp>

#include <variant>

struct EntityHandle;

inline constexpr GLsizei SHADOW_RESOLUTION = 1024;

// Intensity falls off as 1 / (constant + linear * d + quadratic * d^2).
struct Attenuation {
    float constant = 1.0f;
    float linear = 0.0f;
    float quadratic = 0.0f;
};

struct DirectionalLight {};

struct PointLight {
    Attenuation attenuation;
};

struct Spotlight {
    Attenuation attenuation;
    float innerCutoff;
    float outerCutoff;
};

// Color and intensity for every light, plus an on/off switch
struct Light {
    glm::vec3 color{0.0f};
    float intensity;
    bool on = true;
    std::variant<DirectionalLight, PointLight, Spotlight> kind;
    bool castsShadows = true;
    EntityHandle fixture = EntityHandle{};

    // What the shader should receive: black while the light is switched off.
    glm::vec3 effectiveRadiance() const {
        return on ? glm::convertSRGBToLinear(color) * intensity
                  : glm::vec3(0.0f);
    }
};
