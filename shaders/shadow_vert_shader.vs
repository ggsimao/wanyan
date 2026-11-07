#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 4) in mat4 aInstModel;

layout(std140, binding = 0) uniform Matrices {
    mat4 modelMat;
    mat4 projViewMat;
};

uniform int excludedInstance = -1;

void main() {
    if (gl_InstanceID == excludedInstance) {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0); // outside the clip volume
    } else {
        gl_Position = projViewMat * (modelMat * (aInstModel * vec4(aPos, 1.0)));
    }
}
