#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in mat4 aInstModel;
layout(location = 7) in mat3 aInstNormal;

layout(std140, binding = 0) uniform Matrices {
    mat4 modelMat;
    mat4 projViewMat;
    mat4 normalMat;
};

out VERTEX {
    vec3 pos;
    vec3 normal;
    vec2 texCoords;
    vec4 dirLightSpaceFragPos;
    vec4 spotlightSpaceFragPos;
}
vs_out;

void main() {
    vec4 out_pos_4 = modelMat * (aInstModel * vec4(aPos, 1.0));
    vs_out.pos = vec3(out_pos_4);

    vs_out.normal = normalize(mat3(normalMat) * (aInstNormal * aNormal));

    gl_Position = projViewMat * out_pos_4;
}
