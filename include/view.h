#pragma once

#include "data/frame_buffer.h"
#include "data/frame_buffer_multisample.h"
#include "drawables/mesh_buffers.h"
#include "scene/entity_handle.h"
#include "shaders/shader_program.h"

#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

struct Camera;
class SceneRenderer;
class Scene;

inline constexpr float DEFAULT_GAMMA = 2.2f;

struct ViewParameters {
    glm::vec4 clearColor{0.0f};
    bool filterOn = false;
    bool msaaOn = false;
    bool mirror = false;
    float gamma = DEFAULT_GAMMA;
    bool crosshair = false;
};

struct PostShaders {
    const ShaderProgram& resolve;
    const ShaderProgram& luminance;
    const ShaderProgram& tonemap;
};

// An offscreen view of the scene: renders into its own multisampled
// framebuffer, then presents it to the window or onto another screen through
// a fullscreen canvas and the screen shaders (MSAA resolve, border filter, tone
// mapping). The canvas is drawn in clip space, so the screen shader takes its
// transform as a plain `model` uniform rather than from the scene's Matrices
// block.
class View {
  public:
    View(GLsizei width, GLsizei height, const glm::vec4& clearColor);
    View(const View&) = delete;
    View& operator=(const View&) = delete;
    View(View&&) = delete;
    View& operator=(View&&) = delete;

    // Renders the scene, as seen by the camera, into this screen's framebuffer.
    void drawScene(SceneRenderer& renderer, const Scene& scene,
                   EntityHandle cameraHandle) const;
    void postProcess(const PostShaders& shaders) const;
    void present(const ShaderProgram& shader,
                 const DrawFrameBufferBinding& target, const glm::mat4& model);

    const FrameBufferMultisample& scene() const;
    const FrameBuffer& resolved() const;
    const FrameBuffer& luminance() const;
    const FrameBuffer& output() const;

    ViewParameters& parameters();
    const ViewParameters& parameters() const;

  private:
    void drawResolved(const ShaderProgram& shader) const;
    void drawLuminance(const ShaderProgram& shader) const;
    void drawTonemapped(const ShaderProgram& shader) const;

    FrameBufferMultisample _scene;
    FrameBuffer _resolved, _luminance, _output;
    MeshBuffers _canvas;
    ViewParameters _parameters;
};
