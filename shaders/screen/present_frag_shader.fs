#version 430 core
in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D screenTexture;
uniform bool crosshair;
uniform vec2 resolution;  // the window's size in pixels

void main() {
    vec4 pixelColor = texture(screenTexture, texCoords);
    if (crosshair) {
        vec2 fromCenter = abs(gl_FragCoord.xy - resolution * 0.5);
        bool onCross = (fromCenter.x < 1.0 && fromCenter.y < 8.0) ||
                       (fromCenter.y < 1.0 && fromCenter.x < 8.0);
        if (onCross)
            pixelColor.rgb = 1.0 - pixelColor.rgb;
    }
    fragColor = pixelColor;
}
