#pragma once

#include "scene/scene.h"

class Camera;
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
    const ShaderProgram& debug;
};

// Choices about how to draw, not about what's in the scene.
struct RenderOptions {
    bool visualizeNormals = false;
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
    void compose(const Scene& scene, const Camera& camera) const;

    RenderOptions& options();
    const RenderOptions& options() const;

private:
    void drawSkyboxes(const Scene& scene, TextureUnits& units) const;
    void drawObjects(const Scene& scene, const Camera& camera, TextureUnits& units) const;
    void drawShadowCasters(const Scene& scene, const ShaderProgramBinding& shader) const;

    SceneShaders _shaders;
    UniformBuffer& _ubo;
    RenderOptions _options;
};
