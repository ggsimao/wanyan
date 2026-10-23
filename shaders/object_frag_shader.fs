#version 430 core
in VERTEX {
    vec3 pos;
    vec3 normal;
    vec2 texCoords;
    vec4 dirLightSpaceFragPos;
    vec4 spotlightSpaceFragPos;
}
fs_in;

#define NR_DIFFUSE_TEXTURES 3
#define NR_SPECULAR_TEXTURES 3

struct Material {
    sampler2D diffuseTextures[NR_DIFFUSE_TEXTURES];
    sampler2D specularTextures[NR_SPECULAR_TEXTURES];
    float shininess;
    int loadedDiffuse;
    int loadedSpecular;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    sampler2D shadow_map;
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    sampler2D shadow_map;
};

struct Spotlight {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float phiCos;
    float gammaCos;

    sampler2D shadow_map;
};

layout(std140, binding = 0) uniform Matrices {
    mat4 modelMat;
    mat4 projViewMat;
    mat4 normalMat;
};

#define NR_POINT_LIGHTS 4
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform Spotlight spotlight;

#define PCF_KERNEL_SIZE 5

uniform vec3 viewPos;

uniform Material material;

out vec4 fragColor;

float calculateShadowValue(vec4 fragPosLightSpace, sampler2D shadowMap,
                           vec3 lightDir, vec3 normal) {
    vec3 projCoords = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;

    float currentDepth = projCoords.z;

    if (currentDepth > 1.0)
        return 0.0;

    float shadow_bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = 0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    int kernel_half = (PCF_KERNEL_SIZE - 1) / 2;
    for (int x = -kernel_half; x <= kernel_half; ++x) {
        for (int y = -kernel_half; y <= kernel_half; ++y) {
            float pcfDepth =
                texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - shadow_bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= PCF_KERNEL_SIZE * PCF_KERNEL_SIZE;

    return shadow;
}

vec3 calculateLightValue(float diff_str, float spec_str, vec3 amb_color,
                         vec3 diff_color, vec3 spec_color, vec3 diff_tex_sum,
                         vec3 spec_tex_sum, float shadow) {
    vec3 ambient = diff_tex_sum * amb_color;
    vec3 diffuse = diff_tex_sum * diff_color * diff_str;
    vec3 specular = spec_tex_sum * spec_color * spec_str;

    vec3 final_light = ambient + (1 - shadow) * (diffuse + specular);

    return final_light;
}

vec3 calculateDirectionalLight(DirLight light, vec3 normal, vec3 viewDir,
                               vec3 diff_tex_sum, vec3 spec_tex_sum) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    float shadow = calculateShadowValue(fs_in.dirLightSpaceFragPos,
                                        light.shadow_map, lightDir, normal);

    vec3 directional_value =
        calculateLightValue(diff, spec, light.ambient, light.diffuse,
                            light.specular, diff_tex_sum, spec_tex_sum, shadow);

    return directional_value;
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos,
                         vec3 viewDir, vec3 diff_tex_sum, vec3 spec_tex_sum) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist +
                               light.quadratic * (dist * dist));

    float shadow = 0;
    // calculateShadowValue(vec4(fs_in.pos, 1), light.shadow_map, lightDir);

    vec3 pointlight_value =
        calculateLightValue(diff, spec, light.ambient, light.diffuse,
                            light.specular, diff_tex_sum, spec_tex_sum, shadow);
    pointlight_value *= attenuation;

    return pointlight_value;
}

vec3 calculateSpotlight(Spotlight light, vec3 normal, vec3 fragPos,
                        vec3 viewDir, vec3 diff_tex_sum, vec3 spec_tex_sum) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    float theta = dot(lightDir, normalize(-light.direction));
    float intensity =
        clamp((theta - light.gammaCos) / (light.phiCos - light.gammaCos), 0.0, 1.0);

    // float shadow = 0;
    float shadow = calculateShadowValue(fs_in.spotlightSpaceFragPos,
                                        light.shadow_map, lightDir, normal);

    vec3 spotlight_value =
        calculateLightValue(diff, spec, light.ambient, light.diffuse,
                            light.specular, diff_tex_sum, spec_tex_sum, shadow);
    spotlight_value *= intensity;

    return spotlight_value;
}

void main() {
    vec3 viewDir = normalize(viewPos - fs_in.pos);
    vec3 normal = normalize(fs_in.normal);

    vec4 init_diff_tex = texture(material.diffuseTextures[0], fs_in.texCoords);
    vec4 init_spec_tex = texture(material.specularTextures[0], fs_in.texCoords);
    vec3 diff_tex_sum = init_diff_tex.rgb;
    float alpha = init_diff_tex.a;
    vec3 spec_tex_sum = init_spec_tex.rgb;
    for (int i = 1; i < material.loadedDiffuse; i++) {
        vec4 curr_tex = texture(material.diffuseTextures[i], fs_in.texCoords);
        diff_tex_sum = mix(diff_tex_sum, curr_tex.rgb, curr_tex.a);
        alpha = mix(alpha, 1.0, curr_tex.a);
    }
    if (alpha < 0.01) {
        discard;
    }
    for (int i = 1; i < material.loadedSpecular; i++) {
        vec4 curr_tex = texture(material.specularTextures[i], fs_in.texCoords);
        spec_tex_sum = mix(spec_tex_sum, curr_tex.rgb, curr_tex.a);
    }

    vec3 result = calculateDirectionalLight(dirLight, normal, viewDir,
                                            diff_tex_sum, spec_tex_sum);

    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        vec3 pointlight_value =
            calculatePointLight(pointLights[i], normal, fs_in.pos,
                                viewDir, diff_tex_sum, spec_tex_sum);
        result += pointlight_value;
    }

    vec3 spotlight_value =
        calculateSpotlight(spotlight, normal, fs_in.pos, viewDir,
                           diff_tex_sum, spec_tex_sum);
    result += spotlight_value;

    fragColor = vec4(result, alpha);
}
