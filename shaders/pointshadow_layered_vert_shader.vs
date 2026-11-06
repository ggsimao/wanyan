#version 430 core
#extension GL_ARB_shader_viewport_layer_array : require // this is for optimizing point shadows but it's not always present
layout(location = 0) in vec3 aPos;
layout(location = 3) in mat4 aInstModel;

layout(std140, binding = 0) uniform Matrices {
    mat4 modelMat;
};

const int FACES = 6;

uniform int excludedInstance;
uniform mat4 shadowMatrices[6];

out vec4 FragPos;

void main() {
    int instance = gl_InstanceID / FACES;
    if (instance == excludedInstance) {
        FragPos = vec4(0.0);
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0); // outside the clip volume
    } else {
        int face = gl_InstanceID % FACES;
        FragPos = modelMat * (aInstModel * vec4(aPos, 1.0));
        gl_Position = shadowMatrices[face] * FragPos;
        gl_Layer = face;
    }
}
