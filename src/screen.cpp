#include "screen.h"

#include "data/frame_buffer_multisample.h"
#include "render_state/capability.h"
#include "scene/scene_renderer.h"
#include "shaders/shader_program.h"
#include "textures/draw_binding.h"
#include "textures/helpers.h"
#include "textures/texture_2d.h"

#include <glm/gtc/matrix_transform.hpp>

Screen::Screen(GLsizei width, GLsizei height, const glm::vec4& clearColor,
               const ShaderProgram& shader,
               const ShaderProgram& luminanceShader)
    : _framebuffer(width, height),
      _luminanceFramebuffer(256, 256, TextureType::LUMINANCE),
      _canvas(_framebuffer.texture()), _shader(shader),
      _luminanceShader(luminanceShader) {
    Texture2DBinding(_luminanceFramebuffer.texture())
        .setFilters(GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR);
    _parameters.clearColor = clearColor;
}

void Screen::drawOnFramebuffer(SceneRenderer& renderer, const Scene& scene,
                               EntityHandle cameraHandle) const {
    DrawFrameBufferBindingMultisample target(_framebuffer);
    target.clear(_parameters.clearColor);
    renderer.compose(scene, cameraHandle);
}

void Screen::drawLuminance() const {
    {
        DrawFrameBufferBinding target(_luminanceFramebuffer);

        render_state::Capability noDepth(GL_DEPTH_TEST, false);

        TextureUnits units;
        ShaderProgramBinding shader(_luminanceShader);
        shader.setMat4("model", glm::mat4(1.0f));

        _canvas.draw(shader, units);
    }
    Texture2DBinding(_luminanceFramebuffer.texture()).generateMipmap();
}

void Screen::drawOnAnother(const Screen& other, float scaling,
                           const glm::vec2& offset) const {
    DrawFrameBufferBindingMultisample target(other._framebuffer);
    // Scaled about the centre, then moved: what Rust did by cloning the canvas
    // object and transforming it every frame.
    const glm::mat4 model =
        glm::translate(glm::mat4(1.0f), glm::vec3(offset, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(scaling, scaling, 1.0f));
    // Gamma 1: the other screen applies gamma once, when it's presented.
    // The effects are the other screen's, which is what Rust showed: it left
    // them at whatever the last drawOnScreen had set on the shared shader.
    ScreenParameters parameters = other._parameters;
    parameters.gamma = 1.0f;
    parameters.crosshair = false;
    present(model, parameters);
}

void Screen::drawOnScreen() const {
    auto target = DrawFrameBufferBinding::screen(_framebuffer.width(),
                                                 _framebuffer.height());
    target.clear(glm::vec4(1.0f), GL_COLOR_BUFFER_BIT);
    present(glm::mat4(1.0f), _parameters);
}

void Screen::present(const glm::mat4& model,
                     const ScreenParameters& effects) const {
    // The canvas is drawn on top of whatever is there.
    render_state::Capability noDepth(GL_DEPTH_TEST, false);

    TextureUnits units;
    ShaderProgramBinding shader(_shader);
    shader.setMat4("model", model);
    shader.setFloat("gamma", effects.gamma);
    shader.setBool("applyFilter", effects.filterOn);
    shader.setBool("applyMSAA", effects.msaaOn);
    shader.setBool("crosshair", effects.crosshair);
    shader.setVec2("resolution", {_framebuffer.width(), _framebuffer.height()});
    TextureDrawBinding luminanceTexture(units, _luminanceFramebuffer.texture());
    shader.setTexture("luminanceLog2", luminanceTexture);

    _canvas.draw(shader, units);
}

ScreenParameters& Screen::parameters() { return _parameters; }

const ScreenParameters& Screen::parameters() const { return _parameters; }
