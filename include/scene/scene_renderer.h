#pragma once

#include "data/point_shadow_frame_buffer.h"
#include "data/shadow_frame_buffer.h"
#include "scene/entity.h"
#include "scene/frame_data.h"
#include "scene/scene.h"
#include "shaders/point_shadow_shader.h"
#include <map>

class ShaderProgram;
class ShaderProgramBinding;
class TextureUnits;
class UniformBuffer;

// The programs the renderer draws with. Owned elsewhere (e.g. main).
struct SceneShaders {
    const ShaderProgram& object;
    const ShaderProgram& skybox;
    const ShaderProgram& outline;
    const ShaderProgram& shadow;
    const PointShadowShader& pointShadow;
    const ShaderProgram& debug;
};

struct SceneParameters {
    const Transform& cameraTransform;
    const glm::vec3& ambientColor = glm::vec3(0.0f);
};

// Choices about how to draw, not about what's in the scene.
struct RenderOptions {
    bool visualizeNormals = false;
};

struct ShadowMapBindings {
    const TextureDrawBinding<Texture2D>& dirShadows;
    const TextureDrawBinding<Texture2D>& spotShadows;
    const TextureDrawBinding<CubeMap>& pointShadows;
    const TextureDrawBinding<Texture2D>& dummyShadow;
    const TextureDrawBinding<CubeMap>& dummyPointShadow;
};

// How a Scene is drawn: the shadow pass, the skybox pass and the object
// pass, with their shaders. Draws any Scene from
// any Camera, so a mirror is just another compose() call with an inverted
// camera. Sets every piece of GL state it relies on itself, and restores it.
class SceneRenderer {
  public:
    SceneRenderer(SceneShaders shaders, UniformBuffer& ubo);

    // Renders depth from each shadow-casting light into its shadow map.
    // The maps don't depend on the camera: call once per frame, before any
    // compose(). Each map's binding sets and restores framebuffer and viewport.
    void renderShadowMaps(const Scene& scene) const;

    // Draws the scene's skyboxes and objects, as seen by the camera, into the
    // currently bound draw framebuffer.
    void compose(const Scene& scene, EntityHandle activeCamera);

    RenderOptions& options();
    const RenderOptions& options() const;

    void updateFrameData(const Scene& scene);

  private:
    // Calls process(entity) for every active entity,
    // depth-first. Inactive entities are skipped with their whole subtree.
    template <typename F>
    void forEachActive(const Entity& entity, F& process) const {
        if (!entity.activeSelf()) {
            return;
        }
        process(entity);
        for (const Entity* child : entity.getChildren()) {
            forEachActive(*child, process);
        }
    }

    void setLightingUniforms(const ShaderProgramBinding& shader,
                             ShadowMapBindings shadows) const;

    void bindIdentityInstance(const BasicMesh& part) const;

    void drawSkyboxes(const Scene& scene, TextureUnits& units) const;
    void drawObjects(const SceneParameters& parameters,
                     TextureUnits& units) const;
    void drawShadowCasters(const Scene& scene,
                           const ShaderProgramBinding& shader,
                           const std::optional<EntityHandle>& fixture,
                           GLsizei layers) const;

    mutable std::unordered_map<const BasicMesh*, ArrayBuffer> _instanceBuffers;
    FrameData _frameData;
    SceneShaders _shaders;
    UniformBuffer& _ubo;
    RenderOptions _options;
    std::map<EntityHandle, ShadowFrameBuffer> _dirShadowMaps,
        _spotShadowMaps;  // directional, spot
    std::map<EntityHandle, PointShadowFrameBuffer> _pointShadowMaps;  // point
    ShadowFrameBuffer _dummyShadow{1, 1};  // 2D depth, compare mode on
    PointShadowFrameBuffer _dummyPointShadow{1,
                                             1};  // cube depth, compare mode on
};
