#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 3) in mat4 aInstModel;

uniform mat4 model;

void main() { gl_Position = model * aInstModel * vec4(aPos, 1.0); }