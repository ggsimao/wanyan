#version 430 core
in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D screenTexture;
uniform sampler2D luminanceLog2;
uniform float gamma = 2.2;

const float REFERENCE_LOG = log2(0.2);  // what counts as "normal" brightness
const float BRIGHT_ADAPTATION = 1.0;    // above normal: compensate fully
const float DARK_ADAPTATION = 0.2;  // below normal: compensate only a little

float calculateExposure() {
    float avgLog = textureLod(luminanceLog2, vec2(0.5),
                              textureQueryLevels(luminanceLog2) - 1)
                       .r;
    float delta = avgLog - REFERENCE_LOG;
    float strength = delta > 0.0 ? BRIGHT_ADAPTATION : DARK_ADAPTATION;
    float exposure = 1.0 / (9.6 * exp2(REFERENCE_LOG + strength * delta));
    return exposure;
}

void main() {
    vec3 pixelColor = texture(screenTexture, texCoords).rgb;
    pixelColor *= calculateExposure();
    pixelColor = (pixelColor * (2.51 * pixelColor + 0.03)) /
                 (pixelColor * (2.43 * pixelColor + 0.59) + 0.14);  // ACES
    pixelColor = pow(pixelColor, vec3(1.0 / gamma));  // gamma correction
    fragColor = vec4(pixelColor, 1.0);
}
