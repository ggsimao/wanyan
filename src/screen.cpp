#include "screen.h"

#include "render_state/capability.h"
#include "scene/scene_renderer.h"
#include "shaders/shader_program.h"
#include "textures/draw_binding.h"

#include <glm/gtc/matrix_transform.hpp>

Screen::Screen(GLsizei width, GLsizei height, const glm::vec4& clearColor,
               const ShaderProgram& shader)
    : _framebuffer(width, height), _shader(shader) {
    _parameters.clearColor = clearColor;
}

void Screen::drawOnFramebuffer(const SceneRenderer& renderer, const Scene& scene,
                               const Camera& camera) const {
    DrawFrameBufferBinding target(_framebuffer);
    target.clear(_parameters.clearColor);
    renderer.compose(scene, camera);
}

void Screen::drawOnAnother(const Screen& other, float scaling, const glm::vec2& offset) const {
    DrawFrameBufferBinding target(other._framebuffer);
    // Scaled about the centre, then moved: what Rust did by cloning the canvas
    // object and transforming it every frame.
    const glm::mat4 model =
        glm::translate(glm::mat4(1.0f), glm::vec3(offset, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(scaling, scaling, 1.0f));
    // Gamma 1: the other screen applies gamma once, when it's presented.
    // The effects are the other screen's, which is what Rust showed: it left
    // them at whatever the last drawOnScreen had set on the shared shader.
    present(model, 1.0f, other._parameters);
}

void Screen::drawOnScreen() const {
    auto target = DrawFrameBufferBinding::screen();
    target.clear(glm::vec4(1.0f), GL_COLOR_BUFFER_BIT);
    present(glm::mat4(1.0f), _parameters.gamma, _parameters);
}

void Screen::present(const glm::mat4& model, float gamma, const ScreenParameters& effects) const {
    // The canvas is drawn on top of whatever is there.
    render_state::Capability noDepth(GL_DEPTH_TEST, false);

    TextureUnits units;
    ShaderProgramBinding shader(_shader);
    shader.setMat4("model", model);
    TextureDrawBinding screenTexture(units, _framebuffer.texture());
    shader.setTexture("screenTexture", screenTexture);
    shader.setInt("sampleCount", _framebuffer.texture().samples());
    shader.setFloat("gamma", gamma);
    shader.setBool("applySobel", effects.sobelOn);
    shader.setBool("applyMSAA", effects.msaaOn);

    _canvas.draw(shader, units);
}

ScreenParameters& Screen::parameters() {
    return _parameters;
}

const ScreenParameters& Screen::parameters() const {
    return _parameters;
}
