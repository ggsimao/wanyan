#pragma once

#include "random_transform.h"
#include "scene/scene.h"
#include "scene/scene_renderer.h"
#include "shaders/shader_program.h"

#include <cstddef>
#include <vector>

class Camera;

// The world this program shows: which shaders it uses, which lights and
// objects exist and where. Everything specific to the current demo lives in
// this file pair; the rest of the engine doesn't know about rocks or windows.

// The rocks are scene.objects[0], with this many instances.
inline constexpr std::size_t ROCK_INSTANCES = 1000;

// All the programs, owned together. Keep alive for as long as anything draws.
struct DemoShaders {
    ShaderProgram object;
    ShaderProgram debug;
    ShaderProgram outline;
    ShaderProgram screen;
    ShaderProgram skybox;
    ShaderProgram shadow;

    // The references SceneRenderer draws with.
    SceneShaders forRenderer() const;
};

// Throws std::runtime_error if a shader file is missing or doesn't compile.
DemoShaders loadDemoShaders();

// Lights, objects and skybox. The flashlight starts at the camera.
Scene buildDemoScene(const Camera& camera);

// One per rock instance, in the same order.
std::vector<RandomTransform> buildRockTransforms();
