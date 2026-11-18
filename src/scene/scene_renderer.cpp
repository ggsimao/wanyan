#include "scene/scene_renderer.h"

#include "data/instance_buffer.h"
#include "data/point_shadow_frame_buffer.h"
#include "data/shadow_frame_buffer.h"
#include "data/uniform_buffer.h"
#include "render_state/blending.h"
#include "render_state/capability.h"
#include "render_state/color.h"
#include "render_state/culling.h"
#include "render_state/depth.h"
#include "render_state/stencil.h"
#include "scene/entity.h"
#include "scene/instance.h"
#include "shaders/shader_program.h"
#include "shaders/uniform_name.h"
#include "textures/cube_map.h"
#include "textures/draw_binding.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <format>
#include <optional>
#include <ranges>
#include <vector>

namespace {
    constexpr float LAMP_NEAR_PLANE = 0.1f;
    constexpr float LAMP_FAR_PLANE = 100.0f;
    constexpr float CAMERA_NEAR_PLANE = 0.1f;
    constexpr float CAMERA_FAR_PLANE = 100.0f;
    constexpr float SUN_DISTANCE = 20.0f;
    constexpr float ASPECT_RATIO =
        1.0f;  // as in the Rust version, until window resizing
    constexpr size_t FACES = 6;
    constexpr GLuint MAX_DIR = 4, MAX_POINT = 4, MAX_SPOT = 4;
    const glm::vec3 WORLD_UP(0.0f, 1.0f, 0.0f);

    constexpr float OUTLINE_SCALE = 1.1f;

    struct LightSpace {
        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 matrix() const { return projection * view; }
    };

    struct PointLightSpace {
        glm::mat4 projection;
        std::array<glm::mat4, 6> views;
        glm::mat4 matrix(size_t index) const {
            return projection * views[index];
        }
    };

    LightSpace directionalLightSpace(const Entity* entity) {
        // An orthographic box around the origin, looking along the light.
        const glm::vec3 position =
            -glm::normalize(entity->getTransform().getRot() *
                            glm::vec3(0.0f, 0.0f, -1.0f)) *
            SUN_DISTANCE;
        return {
            glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 2.0f * SUN_DISTANCE),
            glm::lookAt(position, glm::vec3(0.0f), WORLD_UP)};
    }

    LightSpace spotlightSpace(const Entity* entity, glm::vec3 dir,
                              float outerCutoff) {
        // The cone's full angle is twice its half-angle cutoff. Rust passed
        // gamma.to_degrees() where radians were expected, and only the
        // half-angle.
        const Transform& transform = entity->getTransform();
        glm::vec3 pos = transform.getPos();
        return {glm::perspective(2.0f * outerCutoff, 1.0f, CAMERA_NEAR_PLANE,
                                 CAMERA_FAR_PLANE),
                glm::lookAt(pos, pos + dir, WORLD_UP)};
    }

    PointLightSpace pointLightSpaces(const Entity* entity) {
        glm::vec3 pos = entity->getTransform().getPos();
        std::array<glm::mat4, 6> views = {
            glm::lookAt(pos, pos + glm::vec3(1.0, 0.0, 0.0),
                        glm::vec3(0.0, -1.0, 0.0)),
            glm::lookAt(pos, pos + glm::vec3(-1.0, 0.0, 0.0),
                        glm::vec3(0.0, -1.0, 0.0)),
            glm::lookAt(pos, pos + glm::vec3(0.0, 1.0, 0.0),
                        glm::vec3(0.0, 0.0, 1.0)),
            glm::lookAt(pos, pos + glm::vec3(0.0, -1.0, 0.0),
                        glm::vec3(0.0, 0.0, -1.0)),
            glm::lookAt(pos, pos + glm::vec3(0.0, 0.0, 1.0),
                        glm::vec3(0.0, -1.0, 0.0)),
            glm::lookAt(pos, pos + glm::vec3(0.0, 0.0, -1.0),
                        glm::vec3(0.0, -1.0, 0.0)),
        };
        glm::mat4 shadowProj = glm::perspective(
            glm::radians(90.0f), 1.0f, LAMP_NEAR_PLANE, LAMP_FAR_PLANE);
        return {shadowProj, views};
    }

    template <typename... Ts> struct overloaded : Ts... {
        using Ts::operator()...;
    };
}  // namespace

SceneRenderer::SceneRenderer(SceneShaders shaders, UniformBuffer& ubo)
    : _shaders(shaders), _ubo(ubo) {
    // The dummies stand in for unused or shadowless slots, so they must read
    // as "nothing in the way": depth 1.0 everywhere.
    ShadowDrawBinding(_dummyShadow).clearDepth();
    PointShadowDrawBinding(_dummyPointShadow).clearDepth();
}

void SceneRenderer::renderShadowMaps() const {
    render_state::Capability depthTest(GL_DEPTH_TEST, true);
    render_state::DepthFunc depthLess(GL_LESS);
    render_state::Capability culling(GL_CULL_FACE, true);
    render_state::CullFace backFaces(
        GL_BACK);  // front-face culling detached shadows where objects cross
                   // the floor, so back faces are culled and the bias handles
                   // acne.

    const auto renderDirectional = [&](const GatheredLight& gathered) {
        ShaderProgramBinding shader(_shaders.shadow);
        EntityHandle handle = gathered.entity->getHandle();
        ShadowDrawBinding target(_dirShadowMaps.at(handle));
        target.clearDepth();
        _ubo.setProjViewMat(gathered.lightSpace);
        drawShadowCasters(shader, gathered.light->fixture, 1);
    };
    const auto renderSpotlight = [&](const GatheredLight& gathered) {
        ShaderProgramBinding shader(_shaders.shadow);
        EntityHandle handle = gathered.entity->getHandle();
        ShadowDrawBinding target(_spotShadowMaps.at(handle));
        target.clearDepth();
        _ubo.setProjViewMat(gathered.lightSpace);
        drawShadowCasters(shader, gathered.light->fixture, 1);
    };
    const auto renderPointLight = [&](const GatheredLight& gathered) {
        ShaderProgramBinding shader(_shaders.pointShadow.program());
        EntityHandle handle = gathered.entity->getHandle();
        PointShadowDrawBinding target(_pointShadowMaps.at(handle));
        target.clearDepth();
        PointLightSpace space = pointLightSpaces(gathered.entity);
        for (size_t i = 0; i < FACES; ++i) {
            UniformName shadowMatrix("shadowMatrices[{}]", i);
            shader.setMat4(std::format("shadowMatrices[{}]", i),
                           space.matrix(i));
        }

        drawShadowCasters(shader, gathered.light->fixture,
                          _shaders.pointShadow.layers());
    };

    for (const auto* list : {&_frameData.dirLights, &_frameData.spotLights,
                             &_frameData.pointLights}) {
        for (const auto& gathered : *list) {
            if (!gathered.light->on)
                continue;
            std::visit(
                overloaded{
                    [&](const DirectionalLight&) {
                        renderDirectional(gathered);
                    },
                    [&](const Spotlight&) { renderSpotlight(gathered); },
                    [&](const PointLight&) { renderPointLight(gathered); },
                },
                gathered.light->kind);
        }
    }
}

void SceneRenderer::drawShadowCasters(const ShaderProgramBinding& shader,
                                      const EntityHandle& fixture,
                                      GLsizei layers) const {
    _ubo.setModelMat(
        glm::identity<glm::mat4>());  // world matrices are in the instances
    for (const auto& [mesh, batch] : _frameData.shadowCasters) {
        if (batch.instances.empty()) {
            continue;
        }
        int excluded = -1;
        if (const auto it = batch.shadowFixtureInstances.find(fixture);
            it != batch.shadowFixtureInstances.end()) {
            excluded = static_cast<int>(it->second);
        }
        shader.setInt("excludedInstance", excluded);
        mesh->bindInstanceBuffer(batch.buffer);
        mesh->drawPositionsInstanced(
            shader, static_cast<GLsizei>(batch.instances.size()), layers);
    }
}

void SceneRenderer::compose(const Scene& scene, EntityHandle activeCamera) {
    // The state every pass below starts from; restored when compose returns.
    render_state::Capability depthTest(GL_DEPTH_TEST, true);
    render_state::DepthFunc depthLess(GL_LESS);
    render_state::Capability culling(GL_CULL_FACE, true);
    render_state::CullFace backFaces(GL_BACK);
    render_state::Capability stencilTest(GL_STENCIL_TEST, true);
    render_state::StencilFunc stencilAlways(GL_ALWAYS, 1, 0xFF);
    render_state::StencilOp stencilKeep(
        GL_KEEP, GL_KEEP,
        GL_KEEP);  // only outlined objects mark it, in drawObjects
    render_state::StencilMask stencilWrites(0xFF);
    render_state::DepthMask depthMaskOn(true);
    // Blending off as baseline, most objects are opaque.
    render_state::Capability blending(GL_BLEND, false);

    TextureUnits units;

    const Entity* camera = scene.get(activeCamera);
    const Camera* lens = camera ? camera->getCamera() : nullptr;
    if (!lens)
        return;
    const Transform& cameraTransform = camera->getTransform();
    glm::mat4 proj = glm::perspective(lens->fovY, ASPECT_RATIO, lens->nearPlane,
                                      lens->farPlane);

    glm::mat4 cam_lookat =
        glm::mat4_cast(glm::conjugate(cameraTransform.getRot())) *
        glm::translate(glm::mat4(1.0f), -cameraTransform.getPos());

    std::ranges::sort(
        _frameData.transparent, std::greater{}, [&](const SingleDraw& d) {
            return glm::distance(d.entity->getTransform().getPos(),
                                 cameraTransform.getPos());
        });

    // The skybox follows the camera's rotation but not its position.
    glm::mat4 skybox_view = glm::mat4(glm::mat3(cam_lookat));
    _ubo.setProjViewMat(proj * skybox_view);
    drawSkyboxes(scene, units);

    _ubo.setProjViewMat(proj * cam_lookat);
    drawObjects({.camera = camera, .ambientColor = scene.ambient}, units);
}

void SceneRenderer::updateFrameData(const Scene& scene) {
    const auto destroyed = [&](const auto& entry) {
        return scene.get(entry.first) == nullptr;
    };
    std::erase_if(_dirShadowMaps, destroyed);
    std::erase_if(_spotShadowMaps, destroyed);
    std::erase_if(_pointShadowMaps, destroyed);
    for (auto& [mesh, batch] : _frameData.opaque) {
        batch.instances.clear();
    }
    for (auto& [mesh, batch] : _frameData.shadowCasters) {
        batch.instances.clear();
        for (auto& [fixture, instance] : batch.shadowFixtureInstances) {
            instance = -1;
        }
    }
    _frameData.transparent.clear();
    _frameData.outlined.clear();
    _frameData.dirLights.clear();
    _frameData.spotLights.clear();
    _frameData.pointLights.clear();
    auto gatherObjects = [&](const Entity& entity) {
        const Light* light = entity.getLight();
        EntityHandle handle = entity.getHandle();
        if (light && light->on) {
            const Transform& transform = entity.getTransform();
            glm::vec3 pos = transform.getPos();
            glm::vec3 dir = transform.getRot() * glm::vec3(0.0f, 0.0f, -1.0f);
            if (std::holds_alternative<PointLight>(light->kind)) {
                if (_frameData.pointLights.size() < MAX_POINT) {
                    _frameData.pointLights.emplace_back(
                        &entity, light, pos, dir, glm::identity<glm::mat4>());
                    _pointShadowMaps.try_emplace(handle, SHADOW_RESOLUTION,
                                                 SHADOW_RESOLUTION);
                }
            } else if (std::holds_alternative<DirectionalLight>(light->kind)) {
                if (_frameData.dirLights.size() < MAX_DIR) {
                    _frameData.dirLights.emplace_back(
                        &entity, light, pos, dir,
                        directionalLightSpace(&entity).matrix());
                    _dirShadowMaps.try_emplace(handle, SHADOW_RESOLUTION,
                                               SHADOW_RESOLUTION);
                }
            } else if (_frameData.spotLights.size() < MAX_SPOT) {
                _frameData.spotLights.emplace_back(
                    &entity, light, pos, dir,
                    spotlightSpace(&entity, dir,
                                   std::get<Spotlight>(light->kind).outerCutoff)
                        .matrix());
                _spotShadowMaps.try_emplace(handle, SHADOW_RESOLUTION,
                                            SHADOW_RESOLUTION);
            }
        }
        const MeshRenderer* renderer = entity.getMeshRenderer();
        if (!renderer) {
            return;
        }
        const glm::mat4& world = entity.getTransform().getModel(Space::World);
        for (const auto& part : renderer->parts) {
            if (part->material().isTransparent()) {
                _frameData.transparent.emplace_back(&entity, part.get());
            } else {
                _frameData
                    .opaque[{.mesh = part.get(), .layer = renderer->layer}]
                    .instances.emplace_back(world);
            }
            if (renderer->outline) {
                _frameData.outlined.emplace_back(&entity, part.get());
            }
            if (renderer->castsShadows) {
                std::vector<Instance>& thisInstances =
                    _frameData.shadowCasters[part.get()].instances;
                _frameData.shadowCasters[part.get()]
                    .shadowFixtureInstances[handle] = thisInstances.size();
                thisInstances.emplace_back(world);
            }
        }
    };
    scene.forEachActive(gatherObjects);
    for (const auto& [_, batch] : _frameData.opaque) {
        InstanceBufferBinding(batch.buffer).upload(batch.instances);
    }
    for (const auto& [_, batch] : _frameData.shadowCasters) {
        InstanceBufferBinding(batch.buffer).upload(batch.instances);
    }
}

void SceneRenderer::setLightingUniforms(const ShaderProgramBinding& shader,
                                        ShadowMapBindings shadows) const {
    GLuint dirCount = 0, spotCount = 0, pointCount = 0;
    const auto setDir = [&](const GatheredLight& gathered) {
        if (dirCount >= MAX_DIR)
            return;
        UniformName dirLight("dirLights[{}]", dirCount);
        shader.setDirectionalLightData(dirLight.view(),
                                       {.light = gathered.light,
                                        .dir = gathered.direction,
                                        .space = gathered.lightSpace});
        UniformName dirLightShadowMap("dirLights[{}].shadow_map", dirCount);
        if (!gathered.light->castsShadows) {
            shader.setTexture(dirLightShadowMap.view(), shadows.dummyShadow, 0);
        } else {
            shader.setTexture(dirLightShadowMap.view(), shadows.dirShadows,
                              dirCount);
        }
        dirCount++;
    };
    const auto setSpot = [&](const GatheredLight& gathered) {
        if (spotCount >= MAX_SPOT)
            return;
        UniformName spotlight("spotlights[{}]", spotCount);
        shader.setSpotlightData(spotlight.view(),
                                {
                                    .light = gathered.light,
                                    .pos = gathered.position,
                                    .dir = gathered.direction,
                                    .nearPlane = CAMERA_NEAR_PLANE,
                                    .farPlane = CAMERA_FAR_PLANE,
                                    .space = gathered.lightSpace,
                                });
        UniformName spotLightShadowMap("spotlights[{}].shadow_map", spotCount);
        if (!gathered.light->castsShadows) {
            shader.setTexture(spotLightShadowMap.view(), shadows.dummyShadow,
                              0);
        } else {
            shader.setTexture(spotLightShadowMap.view(), shadows.spotShadows,
                              spotCount);
        }
        spotCount++;
    };
    const auto setPoint = [&](const GatheredLight& gathered) {
        if (pointCount >= MAX_POINT)
            return;
        UniformName pointLight("pointLights[{}]", pointCount);
        shader.setPointLightData(pointLight.view(),
                                 {.light = gathered.light,
                                  .pos = gathered.position,
                                  .nearPlane = LAMP_NEAR_PLANE,
                                  .farPlane = LAMP_FAR_PLANE});
        UniformName pointLightShadowMap("pointLights[{}].shadow_map",
                                        pointCount);
        if (!gathered.light->castsShadows) {
            shader.setTexture(pointLightShadowMap.view(),
                              shadows.dummyPointShadow, 0);
        } else {
            shader.setTexture(pointLightShadowMap.view(), shadows.pointShadows,
                              pointCount);
        }
        pointCount++;
    };
    for (const auto* list : {&_frameData.dirLights, &_frameData.spotLights,
                             &_frameData.pointLights}) {
        for (const auto& gathered : *list) {
            if (!gathered.light->on)
                continue;
            std::visit(overloaded{
                           [&](const DirectionalLight&) { setDir(gathered); },
                           [&](const Spotlight&) { setSpot(gathered); },
                           [&](const PointLight&) { setPoint(gathered); },
                       },
                       gathered.light->kind);
        }
    }
    // Unused slots still count: GL checks every active sampler at each
    // draw, even ones the shader never reads, and a uniform keeps its last
    // value across frames. So each unused slot points at a dummy of its own
    // type; otherwise a slot left on another type's unit makes every draw
    // fail.
    for (GLuint i = dirCount; i < MAX_DIR; ++i) {
        UniformName slot("dirLights[{}].shadow_map", i);
        shader.setTexture(slot.view(), shadows.dummyShadow, 0);
    }
    for (GLuint i = spotCount; i < MAX_SPOT; ++i) {
        UniformName slot("spotlights[{}].shadow_map", i);
        shader.setTexture(slot.view(), shadows.dummyShadow, 0);
    }
    for (GLuint i = pointCount; i < MAX_POINT; ++i) {
        UniformName slot("pointLights[{}].shadow_map", i);
        shader.setTexture(slot.view(), shadows.dummyPointShadow, 0);
    }
    shader.setInt("loadedDir", dirCount);
    shader.setInt("loadedPoint", pointCount);
    shader.setInt("loadedSpot", spotCount);
}

void SceneRenderer::bindIdentityInstance(const BasicMesh& part) const {
    auto [entry, inserted] = _instanceBuffers.try_emplace(&part);
    ArrayBuffer& buffer = entry->second;
    if (inserted) {
        // Per-entity drawing puts the world matrix in the UBO, so the
        // instance matrix must be the identity.
        const std::array identity{Instance(glm::mat4(1.0f))};
        InstanceBufferBinding(buffer).upload(identity);
    }
    part.bindInstanceBuffer(buffer);
}

void SceneRenderer::drawSkyboxes(const Scene& scene,
                                 TextureUnits& units) const {
    // Seen from inside (no culling), drawn at maximum depth (LEQUAL), and
    // kept out of the stencil buffer.
    render_state::Capability noStencil(GL_STENCIL_TEST, false);
    render_state::Capability noCulling(GL_CULL_FACE, false);
    render_state::DepthFunc depthLequal(GL_LEQUAL);

    ShaderProgramBinding shader(_shaders.skybox);
    for (const Skybox& skybox : scene.skyboxes) {
        skybox.draw(shader, units);
    }
}

void SceneRenderer::drawObjects(const SceneParameters& parameters,
                                TextureUnits& units) const {
    // Opaque objects first, then transparent ones, blended over the
    // finished scene behind them. Both lists are drawn back to front:
    // transparent ones so they blend in the right order, opaque ones so a
    // nearer object can still cover an outline drawn before it.

    const Camera* cam = parameters.camera->getCamera();
    if (!cam)
        return;

    ShaderProgramBinding shader(_shaders.object);

    std::vector<std::reference_wrapper<const Texture2D>> dirShadows;
    for (const GatheredLight& g : _frameData.dirLights) {
        dirShadows.push_back(
            _dirShadowMaps.at(g.entity->getHandle()).texture());
    }
    TextureDrawBinding<Texture2D> dirShadowsBinding(units, dirShadows);

    std::vector<std::reference_wrapper<const Texture2D>> spotShadows;
    for (const GatheredLight& g : _frameData.spotLights) {
        spotShadows.push_back(
            _spotShadowMaps.at(g.entity->getHandle()).texture());
    }
    TextureDrawBinding<Texture2D> spotShadowsBinding(units, spotShadows);

    std::vector<std::reference_wrapper<const CubeMap>> pointShadows;
    for (const GatheredLight& g : _frameData.pointLights) {
        pointShadows.push_back(
            _pointShadowMaps.at(g.entity->getHandle()).texture());
    }
    TextureDrawBinding<CubeMap> pointShadowsBinding(units, pointShadows);

    TextureDrawBinding<Texture2D> dummyShadowBinding(units,
                                                     _dummyShadow.texture());
    TextureDrawBinding<CubeMap> dummyPointShadowBinding(
        units, _dummyPointShadow.texture());

    setLightingUniforms(shader, {dirShadowsBinding, spotShadowsBinding,
                                 pointShadowsBinding, dummyShadowBinding,
                                 dummyPointShadowBinding});
    shader.setVec3("ambientColor", parameters.ambientColor);
    shader.setVec3("viewPos", parameters.camera->getTransform().getPos());

    _ubo.setModelMat(glm::identity<glm::mat4>());
    _ubo.setNormalMat(glm::identity<glm::mat3>());
    for (const auto& [key, batch] : _frameData.opaque) {
        if (batch.instances.empty()) {
            continue;
        }
        if (!(key.layer & cam->mask))
            continue;
        auto instanceCount = static_cast<GLsizei>(batch.instances.size());
        key.mesh->bindInstanceBuffer(batch.buffer);
        key.mesh->drawInstanced(shader, units, instanceCount);
    }
    if (_options.visualizeNormals) {
        ShaderProgramBinding debug(_shaders.debug);
        for (const auto& [key, batch] : _frameData.opaque) {
            if (batch.instances.empty()) {
                continue;
            }
            if (!(key.layer & cam->mask))
                continue;
            auto instanceCount = static_cast<GLsizei>(batch.instances.size());
            key.mesh->bindInstanceBuffer(batch.buffer);
            key.mesh->drawVerticesInstanced(debug, instanceCount);
        }
    }
    {
        ShaderProgramBinding outline(_shaders.outline);
        const auto modelOf = [](const SingleDraw& draw) {
            return draw.entity->getTransform().getModel();
        };
        for (const auto& draw : _frameData.outlined) {
            if (!(draw.entity->getMeshRenderer()->layer & cam->mask))
                continue;
            const bool marks = !draw.mesh->material().isTransparent();
            const glm::mat4 model = modelOf(draw);
            bindIdentityInstance(*draw.mesh);

            if (marks) {
                // Mark: this object's silhouette becomes 1.
                render_state::Capability noDepthTest(GL_DEPTH_TEST, false);
                render_state::ColorMask noColor(false);
                render_state::DepthMask noDepthWrites(false);
                render_state::StencilFunc always(GL_ALWAYS, 1, 0xFF);
                render_state::StencilOp mark(GL_KEEP, GL_KEEP, GL_REPLACE);
                render_state::StencilMask writeStencil(0xFF);
                _ubo.setModelMat(model);
                draw.mesh->drawPlain(outline);
            }
            {
                // Ring: outside this object's mark only.
                render_state::StencilFunc outsideObject(GL_NOTEQUAL, 1, 0xFF);
                render_state::StencilMask freezeStencil(0x00);
                render_state::DepthFunc depthLequal(GL_LEQUAL);
                render_state::DepthMask noDepthWrites(false);
                outline.setVec3(
                    "outlineColor",
                    draw.entity->getMeshRenderer()->outline.value().color);
                _ubo.setModelMat(glm::scale(model, glm::vec3(OUTLINE_SCALE)));
                draw.mesh->drawPlain(outline);
            }
            if (marks) {
                // Unmark: back to 0, so the next object's ring isn't
                // blocked by this one.
                render_state::Capability noDepthTest(GL_DEPTH_TEST, false);
                render_state::ColorMask noColor(false);
                render_state::DepthMask noDepthWrites(false);
                render_state::StencilFunc always(GL_ALWAYS, 1, 0xFF);
                render_state::StencilOp unmark(GL_KEEP, GL_KEEP, GL_ZERO);
                render_state::StencilMask writeStencil(0xFF);
                _ubo.setModelMat(model);
                draw.mesh->drawPlain(outline);
            }
        }
    }
    {
        render_state::Capability blending(GL_BLEND, true);
        render_state::BlendFunc alphaBlend(GL_SRC_ALPHA,
                                           GL_ONE_MINUS_SRC_ALPHA);
        render_state::DepthMask depthMaskOff(false);
        for (const auto& draw : _frameData.transparent) {
            if (!(draw.entity->getMeshRenderer()->layer & cam->mask))
                continue;
            _ubo.setModelMat(draw.entity->getTransform().getModel());
            _ubo.setNormalMat(
                draw.entity->getTransform().getCofactorDetMatrix());
            bindIdentityInstance(*draw.mesh);
            draw.mesh->draw(shader, units);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RenderOptions& SceneRenderer::options() { return _options; }

const RenderOptions& SceneRenderer::options() const { return _options; }
