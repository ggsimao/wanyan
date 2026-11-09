#include "scene/demo_scene.h"

#include "camera.h"
#include "drawables/basic_mesh.h"
#include "drawables/model.h"
#include "drawables/skybox.h"
#include "lighting.h"
#include "random_transform.h"
#include "shaders/shader_program.h"
#include "textures/cube_map.h"
#include "textures/helpers.h"
#include "textures/material.h"
#include "textures/texture_2d.h"

#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>

#include <filesystem>
#include <memory>
#include <random>
#include <utility>

// The project's root directory, so assets are found whatever directory the
// program runs from. CMake defines it; without it, run from the project root.
#ifndef WANYAN_ROOT
#define WANYAN_ROOT "."
#endif

namespace {
    std::filesystem::path asset(const char* relative) {
        return std::filesystem::path(WANYAN_ROOT) / relative;
    }

    constexpr size_t LAMPS_INDEX = 3;

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
    // "resources/textures/awesomeface.png"; constexpr const char* GRASS_TEXTURE
    // = "resources/textures/grass.png"; constexpr const char* ABSTRACT_CUBE =
    // "resources/models/cube/untitled.obj"; constexpr const char* BACKPACK =
    // "resources/models/backpack/backpack.obj";

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

    // ------------------------------------------------------------ lighting

    Lighting buildLighting(const Camera& camera) {
        const glm::vec3 white(1.0f);
        const Attenuation pointAttenuation{
            .constant = 1.0f, .linear = 0.5f, .quadratic = 0.25f};
        const Attenuation flashlightAttenuation{
            .constant = 1.0f, .linear = 0.09f, .quadratic = 0.032f};

        glm::vec3 sun_dir = {1.0f, -2.0f, 1.5f};
        DirectionalLight sun(glm::normalize(sun_dir),
                             {.color = white, .intensity = 1.0f});

        std::vector<PointLight> lamps;
        size_t instanceIndex = 0;
        for (const glm::vec3& position :
             {glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(-1.0f, -2.0f, -1.0f),
              glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 2.0f, -2.5f)}) {
            lamps.emplace_back(
                position,
                Light{.color = white,
                      .intensity = 0.5f,
                      .fixtureIndices =
                          FixtureIndices{.objectID = LAMPS_INDEX,
                                         .instanceID = instanceIndex}},
                pointAttenuation);
            instanceIndex++;
        }

        Spotlight flashlight(
            camera.pos(), camera.dir(), {.color = white, .intensity = 0.7f},
            flashlightAttenuation, glm::radians(15.0f), glm::radians(20.0f));

        glm::vec3 ambientColor(0.02, 0.02, 0.02);

        return Lighting{std::move(sun), std::move(lamps), std::move(flashlight),
                        std::move(ambientColor)};
    }

    // ------------------------------------------------------------- objects

    SceneObject buildRocks() {
        SceneObject rocks(Model(asset(ROCK_MODEL)));
        rocks.scale(glm::vec3(0.1f));
        rocks.addInstances(ROCK_INSTANCES - 1);
        for (std::size_t i = 0; i < ROCK_INSTANCES; ++i) {
            RandomTransform::position(rocks.instance(i), {-100.0f, 100.0f},
                                      {-100.0f, 100.0f}, {-100.0f, 100.0f});
        }
        return rocks;
    }

    SceneObject buildBox() {
        Material material(
            {.diff = {texture(TextureType::DIFFUSE, CONTAINER_TEXTURE,
                              GL_CLAMP_TO_EDGE)},
             .spec = {texture(TextureType::SPECULAR, CONTAINER_SPECULAR,
                              GL_CLAMP_TO_EDGE)},
             .shininess = 8.0f});
        SceneObject box(BasicMesh::cube(1.0f, std::move(material)));
        box.setOutlineColor({0.5f, 0.2f, 0.3f});
        box.enableOutline(true);
        return box;
    }

    SceneObject buildWindow() {
        Material material({.diff = {texture(TextureType::DIFFUSE,
                                            WINDOW_TEXTURE, GL_CLAMP_TO_EDGE)},
                           .spec = {texture(TextureType::SPECULAR,
                                            WINDOW_SPECULAR, GL_CLAMP_TO_EDGE)},
                           .shininess = 8.0f});
        SceneObject window(BasicMesh::square(1.0f, std::move(material)));
        window.setTransparent(true);
        window.translate({0.0f, 1.5f, -1.5f});
        return window;
    }

    // One small glowing cube per point light, at the light's position.
    SceneObject buildLamps(const std::vector<PointLight>& lights) {
        Material material({.diff = {texture(TextureType::DIFFUSE, LAMP_TEXTURE,
                                            GL_CLAMP_TO_EDGE)},
                           .shininess = 32.0f});
        SceneObject lamps(BasicMesh::cube(1.0f, std::move(material)));
        if (lights.size() > 1) {
            lamps.addInstances(lights.size() - 1);
        }
        for (std::size_t i = 0; i < lights.size(); ++i) {
            lamps.instance(i).translate(lights[i].position);
            lamps.instance(i).scale(glm::vec3(0.1f));
        }
        return lamps;
    }

    SceneObject buildFloor() {
        TexturePtr diffuse =
            texture(TextureType::DIFFUSE, WOOD_TEXTURE, GL_REPEAT);
        TexturePtr normal =
            texture(TextureType::NORMAL, WOOD_NORMAL_MAP, GL_REPEAT);
        TexturePtr depth =
            texture(TextureType::DEPTH, WOOD_DEPTH_MAP, GL_REPEAT);
        Material material({.diff = {diffuse},
                           .normalMap = normal,
                           .depthMap = depth,
                           .shininess = 16.0f,
                           .heightScale = 0.02f,
                           .clipParallaxEdges = true});
        SceneObject floor(BasicMesh::square(10.0f, std::move(material)));
        floor.rotate(-glm::half_pi<float>(), {1.0f, 0.0f, 0.0f});
        floor.translate({0.0f, -1.5f, 0.0f});
        return floor;
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

Scene buildDemoScene(const Camera& camera) {
    Lighting lighting = buildLighting(camera);

    std::vector<SceneObject> objects;
    objects.push_back(
        buildRocks());  // index 0: main applies the rock transforms to it
    objects.push_back(buildBox());
    objects.push_back(buildWindow());
    objects.push_back(buildLamps(lighting.points));
    objects.push_back(buildFloor());

    Scene scene{std::move(objects), {}, std::move(lighting)};
    scene.skyboxes.push_back(buildSkybox());
    return scene;
}

std::vector<RandomTransform> buildRockTransforms(std::uint32_t seed) {
    std::mt19937 engine{seed};
    // How often each rock picks a new spin axis and drift direction, in
    // frames; 0 means never, as in Rust.
    std::uniform_int_distribution<unsigned> rate(0, 1000);

    std::vector<RandomTransform> transforms;
    transforms.reserve(ROCK_INSTANCES);
    for (std::size_t i = 0; i < ROCK_INSTANCES; ++i) {
        transforms.emplace_back(0.1f, 0.1f, rate(engine), rate(engine));
    }
    return transforms;
}
