#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 3) in mat4 aInstModel;

layout(std140, binding = 0) uniform Matrices {
    mat4 modelMat;
    mat4 projViewMat;
    mat4 normalMat;
};

void main() {
    gl_Position = projViewMat * (modelMat * (aInstModel * vec4(aPos, 1.0)));
}
