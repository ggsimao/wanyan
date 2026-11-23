#version 430 core
in vec2 texCoords;

out float fragColor;

uniform sampler2DMS screenTexture;
uniform int sampleCount;

vec3 applySampling(ivec2 center, int samples) {
    vec4 pixelColor = vec4(0);
    for (int s = 0; s < samples; s++) {
        pixelColor += texelFetch(screenTexture, center, s) / samples;
    }
    return pixelColor.rgb;
}

void main() {
    ivec2 tex_size = textureSize(screenTexture);
    ivec2 center = ivec2(texCoords * tex_size);
    vec3 pixelColor = applySampling(center, sampleCount);

    float lum = dot(pixelColor, vec3(0.2125, 0.7154, 0.0721));
    fragColor = log2(max(lum, 1e-4));
}
