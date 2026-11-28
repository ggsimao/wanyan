#version 430 core
in vec2 texCoords;

out vec4 fragColor;

uniform sampler2DMS screenTexture;
uniform int sampleCount;
uniform bool applyFilter, applyMSAA;

const int KERNEL_SIZE = 3;

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
    fragColor = vec4(pixelColor, 1.0);
}
