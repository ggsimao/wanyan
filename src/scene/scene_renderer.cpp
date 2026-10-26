#include "scene/scene_renderer.h"

#include "camera.h"
#include "data/shadow_frame_buffer.h"
#include "data/uniform_buffer.h"
#include "render_state/blending.h"
#include "render_state/capability.h"
#include "render_state/culling.h"
#include "render_state/depth.h"
#include "render_state/stencil.h"
#include "scene/scene_object.h"
#include "shaders/shader_program.h"
#include "textures/draw_binding.h"

#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <format>
#include <string>
#include <vector>

namespace {
    constexpr float NEAR_PLANE = 0.1f;
    constexpr float FAR_PLANE = 100.0f;
    constexpr float SUN_DISTANCE = 20.0f;
    constexpr float ASPECT_RATIO = 1.0f;  // as in the Rust version, until window resizing
    const glm::vec3 WORLD_UP(0.0f, 1.0f, 0.0f);

    constexpr float OUTLINE_SCALE = 1.1f;

    struct LightSpace {
        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 matrix() const { return projection * view; }
    };

    LightSpace directionalLightSpace(const DirectionalLight& light) {
        // An orthographic box around the origin, looking along the light.
        const glm::vec3 position = -glm::normalize(light.direction) * SUN_DISTANCE;
        return {glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 2.0f * SUN_DISTANCE),
                glm::lookAt(position, glm::vec3(0.0f), WORLD_UP)};
    }

    LightSpace spotlightSpace(const Spotlight& spot) {
        // The cone's full angle is twice its half-angle cutoff. Rust passed
        // gamma.to_degrees() where radians were expected, and only the half-angle.
        return {glm::perspective(2.0f * spot.outerCutoff, 1.0f, NEAR_PLANE, FAR_PLANE),
                glm::lookAt(spot.position, spot.position + spot.direction, WORLD_UP)};
    }

    void setLightingUniforms(const ShaderProgramBinding& shader, const Lighting& lighting) {
        shader.setDirectionalLight("dirLight", lighting.directional);
        for (std::size_t i = 0; i < lighting.points.size(); ++i) {
            shader.setPointLight(std::format("pointLights[{}]", i), lighting.points[i]);
        }
        shader.setSpotlight("spotlight", lighting.spotlight);
    }
}  // namespace

SceneRenderer::SceneRenderer(SceneShaders shaders, UniformBuffer& ubo)
    : _shaders(shaders), _ubo(ubo) {}

void SceneRenderer::renderShadowMaps(const Scene& scene) const {
    render_state::Capability depthTest(GL_DEPTH_TEST, true);
    render_state::DepthFunc depthLess(GL_LESS);
    render_state::Capability culling(GL_CULL_FACE, true);
    render_state::CullFace frontFaces(GL_FRONT);  // reduces shadow acne

    ShaderProgramBinding shader(_shaders.shadow);

    const auto render = [&](const ShadowFrameBuffer& shadowMap, const LightSpace& space) {
        ShadowDrawBinding target(shadowMap);  // also sets the viewport to the map's size
        target.clearDepth();
        _ubo.setProjViewMat(space.projection * space.view);
        // _ubo.setProjectionMat(space.projection);
        // _ubo.setViewMat(space.view);
        drawShadowCasters(scene, shader);
    };
    render(scene.lighting.directional.shadowMap, directionalLightSpace(scene.lighting.directional));
    render(scene.lighting.spotlight.shadowMap, spotlightSpace(scene.lighting.spotlight));
}

void SceneRenderer::drawShadowCasters(const Scene& scene, const ShaderProgramBinding& shader) const {
    for (const SceneObject& object : scene.objects) {
        _ubo.setModelMat(object.model());
        _ubo.setNormalMat(object.normalMatrix());
        object.drawPlain(shader);
    }
}

void SceneRenderer::compose(const Scene& scene, const Camera& camera) const {
    // The state every pass below starts from; restored when compose returns.
    render_state::Capability depthTest(GL_DEPTH_TEST, true);
    render_state::DepthFunc depthLess(GL_LESS);
    render_state::Capability culling(GL_CULL_FACE, true);
    render_state::CullFace backFaces(GL_BACK);
    render_state::Capability stencilTest(GL_STENCIL_TEST, true);
    render_state::StencilFunc stencilAlways(GL_ALWAYS, 1, 0xFF);
    render_state::StencilOp stencilReplace(GL_KEEP, GL_KEEP, GL_REPLACE);  // objects mark the stencil
    render_state::StencilMask stencilWrites(0xFF);
    render_state::DepthMask depthMaskOn(true);
    // Blending off as baseline, most objects are opaque.
    render_state::Capability blending(GL_BLEND, false);

    TextureUnits units;
    glm::mat4 proj = glm::perspective(camera.fov(), ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

    glm::mat4 cam_lookat = camera.lookAt();
    // The skybox follows the camera's rotation but not its position.
    glm::mat4 skybox_view = glm::mat4(glm::mat3(cam_lookat));
    _ubo.setProjViewMat(proj * skybox_view);
    drawSkyboxes(scene, units);

    _ubo.setProjViewMat(proj * cam_lookat);
    drawObjects(scene, camera, units);
}

void SceneRenderer::drawSkyboxes(const Scene& scene, TextureUnits& units) const {
    // Seen from inside (no culling), drawn at maximum depth (LEQUAL), and kept
    // out of the stencil buffer.
    render_state::Capability noStencil(GL_STENCIL_TEST, false);
    render_state::Capability noCulling(GL_CULL_FACE, false);
    render_state::DepthFunc depthLequal(GL_LEQUAL);

    ShaderProgramBinding shader(_shaders.skybox);
    for (const Skybox& skybox : scene.skyboxes) {
        skybox.draw(shader, units);
    }
}

void SceneRenderer::drawObjects(const Scene& scene, const Camera& camera,
                                TextureUnits& units) const {
    // Opaque objects first, then transparent ones, blended over the finished
    // scene behind them. Both lists are drawn back to front: transparent ones
    // so they blend in the right order, opaque ones so a nearer object can
    // still cover an outline drawn before it. Sorts pointers, not the objects:
    // their order is main's (it finds the rocks by index), and the mirror
    // draws the same objects.
    const glm::vec3 eye = camera.pos();
    const auto distance = [&](const SceneObject* object) {
        return glm::length(eye - glm::vec3(object->model()[3]));
    };

    std::vector<const SceneObject*> opaque;
    std::vector<const SceneObject*> transparent;
    for (const SceneObject& object : scene.objects) {
        (object.isTransparent() ? transparent : opaque).push_back(&object);
    }
    std::ranges::sort(opaque, std::greater{}, distance);
    std::ranges::sort(transparent, std::greater{}, distance);

    ShaderProgramBinding shader(_shaders.object);
    setLightingUniforms(shader, scene.lighting);
    shader.setVec3("viewPos", eye);

    // The shadow maps keep their units for the whole pass; each object's
    // material takes the units after them and gives them back when drawn.
    const Lighting& lighting = scene.lighting;
    TextureDrawBinding directionalShadow(units, lighting.directional.shadowMap.texture());
    TextureDrawBinding spotlightShadow(units, lighting.spotlight.shadowMap.texture());
    shader.setTexture("dirLight.shadow_map", directionalShadow);
    shader.setTexture("spotlight.shadow_map", spotlightShadow);
    shader.setMat4("dirLightSpaceMatrix", directionalLightSpace(lighting.directional).matrix());
    shader.setMat4("spotlightSpaceMatrix", spotlightSpace(lighting.spotlight).matrix());

    const auto drawObject = [&](const SceneObject* object) {
        _ubo.setModelMat(object->model());
        _ubo.setNormalMat(object->normalMatrix());
        object->draw(shader, units);

        if (_options.visualizeNormals) {
            ShaderProgramBinding debug(_shaders.debug);
            object->drawVertices(debug);
        }  // the object program is back in use here

        if (object->hasOutline()) {
            ShaderProgramBinding outline(_shaders.outline);
            _ubo.setModelMat(glm::scale(object->model(), glm::vec3(OUTLINE_SCALE)));
            object->drawOutline(outline, units);
        }
    };

    for (const SceneObject* object : opaque) {
        drawObject(object);
    }

    render_state::Capability blending(GL_BLEND, true);
    render_state::BlendFunc alphaBlend(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    render_state::DepthMask depthMaskOff(false);

    for (const SceneObject* object : transparent) {
        drawObject(object);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RenderOptions& SceneRenderer::options() {
    return _options;
}

const RenderOptions& SceneRenderer::options() const {
    return _options;
}
