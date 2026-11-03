#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 3) in mat4 aInstModel;

layout(std140, binding = 0) uniform Matrices {
    mat4 modelMat;
};

out vec4 FragPos;

uniform int excludedInstance;
uniform mat4 shadowMatrix;

void main() {
    if (gl_InstanceID == excludedInstance) {
        FragPos = vec4(0.0);
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0); // outside the clip volume
    } else {
        FragPos = modelMat * aInstModel * vec4(aPos, 1.0);
        gl_Position = shadowMatrix * FragPos;
    }
}
