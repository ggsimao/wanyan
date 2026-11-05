#include "lighting.h"
#include "data/point_shadow_frame_buffer.h"

DirectionalLight::DirectionalLight(const glm::vec3& lightDirection, const LightColors& colors)
    : LightColors(colors),
      direction(lightDirection),
      shadowMap(SHADOW_RESOLUTION, SHADOW_RESOLUTION) {}

PointLight::PointLight(const glm::vec3& lightPosition, const LightColors& colors,
                       const Attenuation& lightAttenuation)
    : LightColors(colors),
      position(lightPosition),
      attenuation(lightAttenuation),
      shadowMap(SHADOW_RESOLUTION / 4, SHADOW_RESOLUTION / 4) {}

Spotlight::Spotlight(const glm::vec3& lightPosition, const glm::vec3& lightDirection,
                     const LightColors& colors, Attenuation attenuation,
                     float innerCutoffAngle, float outerCutoffAngle)
    : LightColors(colors),
      position(lightPosition),
      direction(lightDirection),
      attenuation(attenuation),
      innerCutoff(innerCutoffAngle),
      outerCutoff(outerCutoffAngle),
      shadowMap(SHADOW_RESOLUTION, SHADOW_RESOLUTION) {}
