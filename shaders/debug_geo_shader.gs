#version 430 core
layout(points) in;
layout(line_strip, max_vertices = 2) out;

in VERTEX {
    vec3 pos;
    vec3 normal;
    vec2 texCoords;
    vec4 dirLightSpaceFragPos;
    vec4 spotlightSpaceFragPos;
}
gs_in[];

layout(std140, binding = 0) uniform Matrices {
    mat4 modelMat;
    mat4 projViewMat;
    mat4 normalMat;
};

const float MAGNITUDE = 0.1;

void generate_line(vec3 pos, vec3 normal) {
    gl_Position = projViewMat * vec4(pos, 1.0);
    EmitVertex();

    gl_Position = projViewMat * (vec4(pos, 1.0) +
                             normalize(vec4(normal, 0.0)) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main() {
    generate_line(gs_in[0].pos, gs_in[0].normal);
}
