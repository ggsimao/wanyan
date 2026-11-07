#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec4 aTangent;
layout(location = 4) in mat4 aInstModel;
layout(location = 8) in mat3 aInstNormal;

layout(std140, binding = 0) uniform Matrices {
    mat4 modelMat;
    mat4 projViewMat;
    mat4 normalMat;
};

uniform mat4 dirLightSpaceMatrix;
uniform mat4 spotlightSpaceMatrix;

out VERTEX {
    vec3 pos;
    vec3 normal;
    vec2 texCoords;
    mat3 TBN;
    vec4 dirLightSpaceFragPos;
    vec4 spotlightSpaceFragPos;
}
vs_out;

void main() {
    vec4 out_pos_4 = modelMat * (aInstModel * vec4(aPos, 1.0));
    vs_out.pos = vec3(out_pos_4);

    vec3 T = normalize(vec3(modelMat * (aInstModel * vec4(aTangent.xyz, 0.0))));
    vec3 N = normalize(mat3(normalMat) * (aInstNormal * aNormal));
    // re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);
    // then retrieve perpendicular vector B with the cross product of T and N
    vec3 B = cross(N, T) * aTangent.w;
    vs_out.TBN = mat3(T, B, N);

    vs_out.normal = N;

    vs_out.texCoords = aTexCoord;

    vs_out.dirLightSpaceFragPos = dirLightSpaceMatrix * vec4(vs_out.pos, 1.0);
    vs_out.spotlightSpaceFragPos = spotlightSpaceMatrix * vec4(vs_out.pos, 1.0);

    gl_Position = projViewMat * out_pos_4;
}
