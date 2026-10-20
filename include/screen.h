#pragma once

#include "data/frame_buffer.h"
#include "drawables/canvas.h"

#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

class Camera;
class SceneRenderer;
class ShaderProgram;
struct Scene;

inline constexpr float DEFAULT_GAMMA = 2.2f;

struct ScreenParameters {
    glm::vec4 clearColor{0.0f};
    bool filterOn = false;
    bool msaaOn = false;
    float gamma = DEFAULT_GAMMA;
};

// An offscreen view of the scene: renders into its own multisampled
// framebuffer, then presents it to the window or onto another screen through
// a fullscreen canvas and the screen shader (MSAA resolve, border filter, gamma).
// The canvas is drawn in clip space, so the screen shader takes its transform
// as a plain `model` uniform rather than from the scene's Matrices block.
class Screen {
public:
    Screen(GLsizei width, GLsizei height, const glm::vec4& clearColor,
           const ShaderProgram& shader);
    // Deleted because _canvas points to framebuffer
    Screen(const Screen&) = delete;
    Screen& operator=(const Screen&) = delete;
    Screen(Screen&&) = delete;
    Screen& operator=(Screen&&) = delete;

    // Renders the scene, as seen by the camera, into this screen's framebuffer.
    void drawOnFramebuffer(const SceneRenderer& renderer, const Scene& scene,
                           const Camera& camera) const;

    // Draws this screen's image into another screen's framebuffer, scaled and
    // offset in clip space (e.g. 0.3 and (0.5, 0.5) for a corner inset).
    void drawOnAnother(const Screen& other, float scaling, const glm::vec2& offset) const;

    // Presents this screen's image in the window.
    void drawOnScreen() const;

    ScreenParameters& parameters();
    const ScreenParameters& parameters() const;

private:
    void present(const glm::mat4& model, float gamma, const ScreenParameters& effects) const;

    FrameBuffer _framebuffer;
    Canvas _canvas;
    const ShaderProgram& _shader;
    ScreenParameters _parameters;
};
