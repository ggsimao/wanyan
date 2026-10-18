#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

// Set by Screen: identity for the full window, scaled and offset for an inset.
uniform mat4 model;

out vec2 texCoords;

void main() {
    gl_Position = model * vec4(aPos.x, aPos.y, 0.0, 1.0);
    texCoords = aTexCoords;
}
