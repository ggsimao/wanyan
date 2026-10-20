#version 430 core
in VERTEX {
    vec3 pos;
    vec3 normal;
    vec2 texCoords;
    vec4 dirLightSpaceFragPos;
    vec4 spotlightSpaceFragPos;
}
fs_in;

out vec4 fragColor;

#define NR_DIFFUSE_TEXTURES 3
#define NR_SPECULAR_TEXTURES 3

struct Material {
    sampler2D diffuseTextures[NR_DIFFUSE_TEXTURES];
    sampler2D specularTextures[NR_SPECULAR_TEXTURES];
    float shininess;
    int loadedDiffuse;
    int loadedSpecular;
};

uniform Material material;
uniform vec3 outlineColor;

void main() {
    float texture_alpha = texture(material.diffuseTextures[0], fs_in.texCoords).a;
    for (int i = 1; i < material.loadedDiffuse; i++) {
        texture_alpha =
            mix(texture_alpha, 1.0, texture(material.diffuseTextures[i], fs_in.texCoords).a);
    }

    if (texture_alpha == 0) {
        discard;
    } else {
        fragColor = vec4(outlineColor, 1.0);
    }
}
