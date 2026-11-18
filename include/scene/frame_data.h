#pragma once

#include "drawables/basic_mesh.h"
#include "scene/entity.h"
#include "scene/instance.h"
#include <unordered_map>

struct BatchKey {
    const BasicMesh* mesh;
    std::uint32_t layer;
    bool operator==(const BatchKey&) const = default;
};

template <> struct std::hash<BatchKey> {
    std::size_t operator()(const BatchKey& key) const noexcept {
        const std::size_t meshHash = std::hash<const BasicMesh*>{}(key.mesh);
        const std::size_t layerHash = std::hash<std::uint32_t>{}(key.layer);
        // Mixes the second hash in so that swapped values don't collide.
        return meshHash ^
               (layerHash + 0x9e3779b9 + (meshHash << 6) + (meshHash >> 2));
    }
};

struct Batch {
    std::vector<Instance> instances;  // rebuilt each frame
    std::map<EntityHandle, int> shadowFixtureInstances;
    ArrayBuffer buffer;  // this batch's instance buffer
};

struct SingleDraw {  // drawn on its own, with the UBO's model matrix
    const Entity* entity;
    const BasicMesh* mesh;
};

struct GatheredLight {
    const Entity* entity;
    const Light* light;
    glm::vec3 position;    // world position
    glm::vec3 direction;   // world rotation * -Z
    glm::mat4 lightSpace;  // projection * view; directional and spot only
};

struct FrameData {
    std::unordered_map<BatchKey, Batch> opaque;  // grouped by mesh
    std::unordered_map<const BasicMesh*, Batch>
        shadowCasters;                    // grouped by mesh, castsShadows only
    std::vector<SingleDraw> transparent;  // sorted back to front
    std::vector<SingleDraw> outlined;
    std::vector<GatheredLight> dirLights, spotLights, pointLights;
};
