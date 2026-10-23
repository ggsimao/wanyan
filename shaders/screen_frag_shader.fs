#version 430 core
in vec2 texCoords;

out vec4 fragColor;

uniform sampler2DMS screenTexture;
uniform int sampleCount;
uniform bool applyFilter, applyMSAA;
uniform float gamma;

const int KERNEL_SIZE = 3;

const float kernel[KERNEL_SIZE][KERNEL_SIZE] =
    float[][](float[](2, 2, 2), float[](2, -15, 2), float[](2, 2, 2));

void main() {
    ivec2 tex_size = textureSize(screenTexture);
    ivec2 center = ivec2(texCoords * tex_size);
    fragColor = vec4(0);
    if (applyFilter && applyMSAA) {
        for (int s = 0; s < sampleCount; s++) {
            vec4 sampleColor = vec4(0);
            for (int i = 0; i < KERNEL_SIZE; i++) {
                for (int j = 0; j < KERNEL_SIZE; j++) {
                    ivec2 texelCoords = clamp(center + ivec2(i - 1, j - 1), ivec2(0, 0), tex_size - ivec2(1, 1));
                    sampleColor += texelFetch(screenTexture, texelCoords, s) *
                                   kernel[i][j];
                }
            }
            fragColor += sampleColor / sampleCount;
        }

    } else if (applyFilter && !applyMSAA) {
        for (int i = 0; i < KERNEL_SIZE; i++) {
            for (int j = 0; j < KERNEL_SIZE; j++) {
                ivec2 texelCoords = clamp(center + ivec2(i - 1, j - 1), ivec2(0, 0), tex_size - ivec2(1, 1));
                fragColor +=
                    texelFetch(screenTexture, texelCoords, 0) * kernel[i][j];
            }
        }
    } else if (applyMSAA) {
        for (int s = 0; s < sampleCount; s++) {
            vec4 sampleColor = texelFetch(screenTexture, center, s);
            fragColor += sampleColor / sampleCount;
        }
    } else {
        fragColor = texelFetch(screenTexture, center, 0);
    }
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / gamma));
}
