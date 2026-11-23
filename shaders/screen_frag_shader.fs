#version 430 core
in vec2 texCoords;

out vec4 fragColor;

uniform sampler2DMS screenTexture;
uniform sampler2D luminanceLog2;
uniform int sampleCount;
uniform bool applyFilter, applyMSAA, crosshair;
uniform float gamma;
uniform vec2 resolution;  // the window's size in pixels

const int KERNEL_SIZE = 3;
const float REFERENCE_LOG = log2(0.2);  // what counts as "normal" brightness
const float BRIGHT_ADAPTATION = 1.0;    // above normal: compensate fully
const float DARK_ADAPTATION = 0.2;  // below normal: compensate only a little

const float kernel[KERNEL_SIZE][KERNEL_SIZE] =
    float[][](float[](2, 2, 2), float[](2, -15, 2), float[](2, 2, 2));

vec4 applyFiltering(ivec2 tex_size, ivec2 center, int samples) {
    vec4 pixelColor = vec4(0);
    for (int i = 0; i < KERNEL_SIZE; i++) {
        for (int j = 0; j < KERNEL_SIZE; j++) {
            ivec2 texelCoords = clamp(center + ivec2(i - 1, j - 1), ivec2(0, 0),
                                      tex_size - ivec2(1, 1));
            pixelColor +=
                texelFetch(screenTexture, texelCoords, samples) * kernel[i][j];
        }
    }
    return pixelColor;
}

vec3 applySampling(ivec2 tex_size, ivec2 center, int samples,
                   bool shouldFilter) {
    vec4 pixelColor = vec4(0);
    for (int s = 0; s < samples; s++) {
        if (shouldFilter)
            pixelColor += applyFiltering(tex_size, center, s) / samples;
        else
            pixelColor += texelFetch(screenTexture, center, s) / samples;
    }
    return pixelColor.rgb;
}

float calculateExposure() {
    float avgLum = exp2(textureLod(luminanceLog2, vec2(0.5),
                                   textureQueryLevels(luminanceLog2) - 1)
                            .r);
    float delta = log2(avgLum) - REFERENCE_LOG;
    float strength = delta > 0.0 ? BRIGHT_ADAPTATION : DARK_ADAPTATION;
    float exposure = 1.0 / (9.6 * exp2(REFERENCE_LOG + strength * delta));
    return exposure;
}

void main() {
    ivec2 tex_size = textureSize(screenTexture);
    ivec2 center = ivec2(texCoords * tex_size);
    vec3 pixelColor = vec3(0);
    if (applyFilter && applyMSAA) {
        pixelColor = applySampling(tex_size, center, sampleCount, true);
    } else if (applyFilter && !applyMSAA) {
        pixelColor = applySampling(tex_size, center, 1, true);
    } else if (applyMSAA) {
        pixelColor = applySampling(tex_size, center, sampleCount, false);
    } else {
        pixelColor = applySampling(tex_size, center, 1, false);
    }
    pixelColor *= calculateExposure();
    pixelColor = pixelColor / (1.0 + pixelColor);
    pixelColor = pow(pixelColor, vec3(1.0 / gamma));  // gamma correction
    if (crosshair) {
        vec2 fromCenter = abs(gl_FragCoord.xy - resolution * 0.5);
        bool onCross = (fromCenter.x < 1.0 && fromCenter.y < 8.0) ||
                       (fromCenter.y < 1.0 && fromCenter.x < 8.0);
        if (onCross)
            pixelColor = 1.0 - pixelColor;
    }
    fragColor = vec4(pixelColor, 1.0);
}
