#pragma once

#include "drawables/basic_mesh.h"
#include "scene/entity.h"
#include "scene/instance.h"
#include <unordered_map>

struct Batch {
    std::vector<Instance> instances;  // rebuilt each frame
    ArrayBuffer buffer;               // this batch's instance buffer
};

struct SingleDraw {  // drawn on its own, with the UBO's model matrix
    const Entity* entity;
    const BasicMesh* mesh;
    float distance;  // to the camera, for sorting transparent parts
};

struct GatheredLight {
    const Entity* entity;
    const Light* light;
    glm::vec3 position;    // world position
    glm::vec3 direction;   // world rotation * -Z
    glm::mat4 lightSpace;  // projection * view; directional and spot only
};

struct FrameData {
    std::unordered_map<const BasicMesh*, Batch> opaque;  // grouped by mesh
    std::unordered_map<const BasicMesh*, Batch>
        shadowCasters;                    // grouped by mesh, castsShadows only
    std::vector<SingleDraw> transparent;  // sorted back to front
    std::vector<SingleDraw> outlined;
    std::vector<GatheredLight> dirLights, spotLights, pointLights;
};
