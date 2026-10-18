#include "lighting.h"

DirectionalLight::DirectionalLight(const glm::vec3& lightDirection, const LightColors& colors)
    : LightColors(colors),
      direction(lightDirection),
      shadowMap(SHADOW_RESOLUTION, SHADOW_RESOLUTION) {}

PointLight::PointLight(const glm::vec3& lightPosition, const LightColors& colors,
                       const Attenuation& lightAttenuation)
    : LightColors(colors), position(lightPosition), attenuation(lightAttenuation) {}

Spotlight::Spotlight(const glm::vec3& lightPosition, const glm::vec3& lightDirection,
                     const LightColors& colors, float innerCutoffAngle, float outerCutoffAngle)
    : LightColors(colors),
      position(lightPosition),
      direction(lightDirection),
      innerCutoff(innerCutoffAngle),
      outerCutoff(outerCutoffAngle),
      shadowMap(SHADOW_RESOLUTION, SHADOW_RESOLUTION) {}
