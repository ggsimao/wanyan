#pragma once

#include "drawables/skybox.h"
#include "lighting.h"
#include "scene/scene_object.h"

#include <vector>

// What exists in the world. Plain data: SceneRenderer decides how it's drawn.
// Owns its contents, so it's move-only (they hold GL resources).
//
//     Scene scene{std::move(objects), {}, std::move(lighting)};
//     scene.skyboxes.push_back(std::move(skybox));
//
// (Skyboxes are added afterwards because a braced list can only copy, and a
// Skybox can't be copied.)
struct Scene {
    std::vector<SceneObject> objects;
    std::vector<Skybox> skyboxes;
    Lighting lighting;
};
