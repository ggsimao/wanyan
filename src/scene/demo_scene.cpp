#include "scene/demo_scene.h"

#include "components/behaviours/flashlight.h"
#include "components/behaviours/fly_camera.h"
#include "components/behaviours/pickable.h"
#include "components/behaviours/picker.h"
#include "components/behaviours/rock_drift.h"
#include "components/mesh_renderer.h"

#include "drawables/basic_mesh.h"
#include "drawables/model.h"
#include "drawables/skybox.h"

#include "random.h"

#include "shaders/shader_program.h"

#include "textures/cube_map.h"
#include "textures/helpers.h"
#include "textures/material.h"
#include "textures/texture_2d.h"

#include <cmath>
#include <format>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>

#include <filesystem>
#include <memory>
#include <random>
#include <utility>
#include <vector>

// The project's root directory, so assets are found whatever directory the
// program runs from. CMake defines it; without it, run from the project root.
#ifndef WANYAN_ROOT
#define WANYAN_ROOT "."
#endif

namespace {
    std::filesystem::path asset(const char* relative) {
        return std::filesystem::path(WANYAN_ROOT) / relative;
    }

    // constexpr size_t LAMPS_INDEX = 3;

    // Shaders
    constexpr const char* REGULAR_VERT_SHADER =
        "shaders/regular_vert_shader.vs";
    constexpr const char* OBJECT_FRAG_SHADER = "shaders/object_frag_shader.fs";
    constexpr const char* DEBUG_VERT_SHADER = "shaders/debug_vert_shader.vs";
    constexpr const char* DEBUG_GEO_SHADER = "shaders/debug_geo_shader.gs";
    constexpr const char* DEBUG_FRAG_SHADER = "shaders/debug_frag_shader.fs";
    constexpr const char* BUFFER_FRAG_SHADER = "shaders/buffer_frag_shader.fs";
    constexpr const char* SCREEN_VERT_SHADER = "shaders/screen_vert_shader.vs";
    constexpr const char* SCREEN_FRAG_SHADER = "shaders/screen_frag_shader.fs";
    constexpr const char* SKYBOX_VERT_SHADER = "shaders/skybox_vert_shader.vs";
    constexpr const char* SKYBOX_FRAG_SHADER = "shaders/skybox_frag_shader.fs";
    constexpr const char* SHADOW_VERT_SHADER = "shaders/shadow_vert_shader.vs";
    constexpr const char* SHADOW_FRAG_SHADER = "shaders/shadow_frag_shader.fs";
    constexpr const char* POINTSHADOW_LAYERED_VERT_SHADER =
        "shaders/pointshadow_layered_vert_shader.vs";
    constexpr const char* POINTSHADOW_VERT_SHADER =
        "shaders/pointshadow_vert_shader.vs";
    constexpr const char* POINTSHADOW_GEO_SHADER =
        "shaders/pointshadow_geo_shader.gs";
    constexpr const char* POINTSHADOW_FRAG_SHADER =
        "shaders/pointshadow_frag_shader.fs";

    // Textures and models
    constexpr const char* CONTAINER_TEXTURE =
        "resources/textures/container2.png";
    constexpr const char* CONTAINER_SPECULAR =
        "resources/textures/container2_specular.png";
    constexpr const char* LAMP_TEXTURE = "resources/textures/glowstone.png";
    constexpr const char* WINDOW_TEXTURE = "resources/textures/window_diff.png";
    constexpr const char* WINDOW_SPECULAR =
        "resources/textures/window_spec.png";
    constexpr const char* WOOD_TEXTURE = "resources/textures/wood.jpg";
    constexpr const char* WOOD_NORMAL_MAP =
        "resources/textures/wood_normal.png";
    constexpr const char* WOOD_DEPTH_MAP = "resources/textures/wood_depth.png";

    constexpr const char* ROCK_MODEL = "resources/models/rocks/rock.obj";

    // Not used yet; kept for later. Commented out because unused constants
    // are a warning, and -Werror makes that an error.
    // constexpr const char* WALL_TEXTURE = "resources/textures/wall.jpg";
    // constexpr const char* FACE_TEXTURE =
    // "resources/textures/awesomeface.png"; constexpr const char*
    // GRASS_TEXTURE = "resources/textures/grass.png"; constexpr const char*
    // ABSTRACT_CUBE = "resources/models/cube/untitled.obj"; constexpr const
    // char* BACKPACK = "resources/models/backpack/backpack.obj";

    const CubeMapPaths SKYBOX_FACES = {
        asset("resources/textures/skybox/right.jpg"),
        asset("resources/textures/skybox/left.jpg"),
        asset("resources/textures/skybox/top.jpg"),
        asset("resources/textures/skybox/bottom.jpg"),
        asset("resources/textures/skybox/front.jpg"),
        asset("resources/textures/skybox/back.jpg"),
    };

    TexturePtr texture(TextureType type, const char* path, GLenum wrapping) {
        return std::make_shared<const Texture2D>(type, asset(path), wrapping);
    }

    Skybox buildSkybox() {
        CubeMap cubeMap(TextureType::DIFFUSE, SKYBOX_FACES, GL_CLAMP_TO_EDGE);
        {
            CubeMapBinding binding(cubeMap);
            // Mipmapped minification relies on CubeMap::load generating the
            // mipmaps; without them this filter leaves the cube map black.
            binding.setFilters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
        }
        return Skybox(std::move(cubeMap));
    }
}  // namespace

SceneShaders DemoShaders::forRenderer() const {
    return {object, skybox, outline, shadow, pointShadow, debug};
}

DemoShaders loadDemoShaders(const GpuFeatures& gpuFeatures) {
    PointShadowShader pointShadow =
        gpuFeatures.layeredVertexOutput
            ? PointShadowShader::layered("PointShadow",
                                         asset(POINTSHADOW_LAYERED_VERT_SHADER),
                                         asset(POINTSHADOW_FRAG_SHADER))
            : PointShadowShader::withGeometryShader(
                  "PointShadow", asset(POINTSHADOW_VERT_SHADER),
                  asset(POINTSHADOW_GEO_SHADER),
                  asset(POINTSHADOW_FRAG_SHADER));
    return {
        ShaderProgram::fromFiles("Object", asset(REGULAR_VERT_SHADER),
                                 asset(OBJECT_FRAG_SHADER)),
        ShaderProgram::fromFiles("Debug", asset(DEBUG_VERT_SHADER),
                                 asset(DEBUG_GEO_SHADER),
                                 asset(DEBUG_FRAG_SHADER)),
        ShaderProgram::fromFiles("Buffer", asset(REGULAR_VERT_SHADER),
                                 asset(BUFFER_FRAG_SHADER)),
        ShaderProgram::fromFiles("Screen", asset(SCREEN_VERT_SHADER),
                                 asset(SCREEN_FRAG_SHADER)),
        ShaderProgram::fromFiles("Skybox", asset(SKYBOX_VERT_SHADER),
                                 asset(SKYBOX_FRAG_SHADER)),
        ShaderProgram::fromFiles("Shadow", asset(SHADOW_VERT_SHADER),
                                 asset(SHADOW_FRAG_SHADER)),
        std::move(pointShadow),
    };
}

std::vector<EntityHandle> buildDemoScene(Scene& scene) {
    scene.ambient = {0.02, 0.02, 0.02};

    std::vector<EntityHandle> entities;

    /// Light

    const glm::vec3 white(1.0f);
    const Attenuation pointAttenuation{
        .constant = 1.0f, .linear = 0.5f, .quadratic = 0.25f};
    const Attenuation flashlightAttenuation{
        .constant = 1.0f, .linear = 0.09f, .quadratic = 0.032f};

    /// Camera

    glm::vec3 cameraPos = {0.0f, 0.0f, -2.0f};

    auto camera = scene.create(
        {.name = "Camera",
         .pos = cameraPos,
         .rot = glm::quatLookAt(glm::normalize(-cameraPos),
                                glm::vec3(0.0f, 1.0f, 0.0f)),
         .camera = Camera{},
         .light = Light{.color = white,
                        .intensity = 0.7f,
                        .on = false,
                        .kind = Spotlight{.attenuation = flashlightAttenuation,
                                          .innerCutoff = glm::radians(15.0f),
                                          .outerCutoff = glm::radians(20.0f)},
                        .castsShadows = true},
         .behaviours = {FlyCamera{}, Flashlight{}, Picker{}}});
    auto mirror_camera = scene.create({
        .name = "Mirror camera",
        .parent = camera,
        .rot = glm::quat(0.0f, 0.0f, 1.0f, 0.0f),
        .camera = Camera{},
    });

    scene.setActiveCamera(camera);

    entities.push_back((camera));
    entities.push_back((mirror_camera));

    /// Sun

    entities.push_back(scene.create(
        {.name = "Sun",
         .pos = glm::vec3(0.0f),
         .rot = glm::quatLookAt(glm::normalize(glm::vec3(1.0f, -2.0f, 1.5f)),
                                glm::vec3(0.0f, 1.0f, 0.0f)),
         .light = Light{.color = white,
                        .intensity = 1.0f,
                        .on = true,
                        .kind = DirectionalLight{},
                        .castsShadows = true}}));

    /// Box

    const auto cube = BasicMesh::cube(1.0f, Material({}));
    auto cubeBuffers = cube.buffers();
    Material boxMaterial(
        {.diff = {texture(TextureType::DIFFUSE, CONTAINER_TEXTURE,
                          GL_CLAMP_TO_EDGE)},
         .spec = {texture(TextureType::SPECULAR, CONTAINER_SPECULAR,
                          GL_CLAMP_TO_EDGE)},
         .shininess = 8.0f});
    auto boxMesh = std::make_shared<const BasicMesh>(cubeBuffers, boxMaterial);

    entities.push_back(scene.create(
        {.name = "Box",
         .meshRenderer =
             MeshRenderer{.parts = {boxMesh},
                          .outline = Outline{.color = {0.5f, 0.2f, 0.3f}}},
         .behaviours = {Spin{}, Pickable(0.5f)}}));

    /// Lamps

    Material lampMaterial({.diff = {texture(TextureType::DIFFUSE, LAMP_TEXTURE,
                                            GL_CLAMP_TO_EDGE)},
                           .shininess = 32.0f});
    auto lampMesh =
        std::make_shared<const BasicMesh>(cubeBuffers, lampMaterial);
    PointLight lampLight = PointLight{.attenuation = pointAttenuation};

    entities.push_back(scene.create({
        .name = "Lamp 1",
        .pos = glm::vec3(0.0f, 2.0f, 0.0f),
        .scale = glm::vec3(0.1f),
        .meshRenderer = MeshRenderer{.parts = {lampMesh}},
        .light = Light{.color = white,
                       .intensity = 0.5f,
                       .on = true,
                       .kind = lampLight,
                       .castsShadows = true},
        .behaviours = {Pickable(0.05f)},
    }));
    entities.push_back(scene.create({
        .name = "Lamp 2",
        .pos = glm::vec3(-1.0f, -2.0f, -1.0f),
        .scale = glm::vec3(0.1f),
        .meshRenderer = MeshRenderer{.parts = {lampMesh}},
        .light = Light{.color = white,
                       .intensity = 0.5f,
                       .on = true,
                       .kind = lampLight,
                       .castsShadows = true},
        .behaviours = {Pickable(0.05f)},
    }));
    entities.push_back(scene.create({
        .name = "Lamp 3",
        .pos = glm::vec3(1.0f, 0.0f, 1.0f),
        .scale = glm::vec3(0.1f),
        .meshRenderer = MeshRenderer{.parts = {lampMesh}},
        .light = Light{.color = white,
                       .intensity = 0.5f,
                       .on = true,
                       .kind = lampLight,
                       .castsShadows = true},
        .behaviours = {Pickable(0.05f)},
    }));
    entities.push_back(scene.create({
        .name = "Lamp 4",
        .pos = glm::vec3(0.0f, 2.0f, -2.5f),
        .scale = glm::vec3(0.1f),
        .meshRenderer = MeshRenderer{.parts = {lampMesh}},
        .light = Light{.color = white,
                       .intensity = 0.5f,
                       .on = true,
                       .kind = lampLight,
                       .castsShadows = true},
        .behaviours = {Pickable(0.05f)},
    }));

    /// Rocks

    std::uniform_int_distribution<unsigned> rate(0, 1000);
    auto rockModel = Model(asset(ROCK_MODEL));
    for (std::size_t i = 0; i < ROCK_INSTANCES; ++i) {
        const unsigned angularRate = rate(random_engine::engine());
        const unsigned linearRate = rate(random_engine::engine());
        entities.push_back(scene.create(
            {.name = std::format("Rock {}", i),
             .pos = glm::vec3{randomIn({.min = -10.0, .max = 10.0}),
                              randomIn({.min = -10.0, .max = 10.0}),
                              randomIn({.min = -10.0, .max = 10.0})},
             .scale = glm::vec3(0.1f),
             .meshRenderer = MeshRenderer{.parts = rockModel.meshes()},
             .behaviours = {RockDrift(0.1f, 0.01f, angularRate, linearRate),
                            Pickable(0.1f)}}));
    }

    /// Window

    entities.push_back(scene.create(
        {.name = "Window",
         .pos = {0.0f, 1.5f, -1.5f},
         .meshRenderer =
             MeshRenderer{.parts = {std::make_shared<const BasicMesh>(
                              BasicMesh::square(
                                  1.0f, Material({.diff = {texture(
                                                      TextureType::DIFFUSE,
                                                      WINDOW_TEXTURE,
                                                      GL_CLAMP_TO_EDGE)},
                                                  .spec = {texture(
                                                      TextureType::SPECULAR, WINDOW_SPECULAR, GL_CLAMP_TO_EDGE)},
                                                  .shininess = 8.0f,
                                                  .transparent = true})))}},
         .behaviours = {Pickable(0.5f)}}));

    /// Floor

    entities.push_back(scene.create(
        {.name = "Floor",
         .pos = {0.0f, -1.5f, 0.0f},
         .rot = {0.7071068f, -0.7071068f, 0.0f, 0.0f},
         .meshRenderer = MeshRenderer{
             .parts = {std::make_shared<const BasicMesh>(BasicMesh::square(
                 10.0f, Material({
                            .diff = {texture(TextureType::DIFFUSE, WOOD_TEXTURE,
                                             GL_REPEAT)},
                            .normalMap = texture(TextureType::NORMAL,
                                                 WOOD_NORMAL_MAP, GL_REPEAT),
                            .depthMap = texture(TextureType::DEPTH,
                                                WOOD_DEPTH_MAP, GL_REPEAT),
                            .shininess = 16.0f,
                            .heightScale = 0.02f,
                            .clipParallaxEdges = true,
                        })))}}}));

    scene.skyboxes.push_back(buildSkybox());

    return entities;
}
