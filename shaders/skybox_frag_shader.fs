#version 430 core
out vec4 fragColor;

in vec3 texCoords;

uniform samplerCube skybox;
uniform float intensity = 1.0f;

void main() { fragColor = texture(skybox, texCoords) * intensity; }
