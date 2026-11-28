#version 430 core
in vec2 texCoords;

out float fragColor;

uniform sampler2D screenTexture;

void main() {
    vec3 pixelColor = texture(screenTexture, texCoords).rgb;

    float lum = dot(pixelColor, vec3(0.2125, 0.7154, 0.0721));
    if (isnan(lum) || isinf(lum))
        lum = 0.0;
    fragColor = log2(max(lum, 1e-4));
}
