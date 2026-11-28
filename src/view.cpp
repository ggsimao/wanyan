#include "view.h"

#include "data/frame_buffer.h"
#include "data/frame_buffer_multisample.h"
#include "drawables/geometry.h"
#include "render_state/capability.h"
#include "scene/scene_renderer.h"
#include "shaders/shader_program.h"
#include "textures/draw_binding.h"
#include "textures/helpers.h"
#include "textures/texture_2d.h"

#include <glm/gtc/matrix_transform.hpp>

namespace {
    MeshBuffers makeCanvasBuffers() {
        const Geometry quad = quadGeometry(2.0f);
        return MeshBuffers(quad.vertices, quad.indices,
                           {VERTEX_ATTRIBUTES[POS_LOCATION],
                            VERTEX_ATTRIBUTES[TEX_COORDS_LOCATION]});
    }

    constexpr GLsizei LUMINANCE_WIDTH = 256, LUMINANCE_HEIGHT = 256;
}  // namespace

View::View(GLsizei width, GLsizei height, const glm::vec4& clearColor)
    : _scene(width, height),
      _resolved(width, height, TextureType::HDR_ATTACHMENT),
      _luminance(LUMINANCE_WIDTH, LUMINANCE_HEIGHT, TextureType::LUMINANCE),
      _output(width, height), _canvas(makeCanvasBuffers()) {
    Texture2DBinding(_luminance.texture())
        .setFilters(GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR);
    _parameters.clearColor = clearColor;
}

void View::drawScene(SceneRenderer& renderer, const Scene& scene,
                     EntityHandle cameraHandle) const {
    DrawFrameBufferBindingMultisample target(_scene);
    target.clear(_parameters.clearColor);
    renderer.compose(scene, cameraHandle);
}

void View::postProcess(const PostShaders& shaders) const {
    drawResolved(shaders.resolve);
    drawLuminance(shaders.luminance);
    drawTonemapped(shaders.tonemap);
}

void View::present(const ShaderProgram& shader,
                   const DrawFrameBufferBinding& target,
                   const glm::mat4& model) {
    target.assertActive();

    // The canvas is drawn on top of whatever is there.
    render_state::Capability noDepth(GL_DEPTH_TEST, false);

    TextureUnits units;
    const ShaderProgramBinding shaderBinding(shader);
    shaderBinding.setMat4("model", model);
    shaderBinding.setBool("crosshair", _parameters.crosshair);
    shaderBinding.setVec2("resolution", {target.width(), target.height()});
    TextureDrawBinding screen(units, _output.texture());
    shaderBinding.setTexture("screenTexture", screen);

    _canvas.draw();
}

void View::drawResolved(const ShaderProgram& shader) const {
    DrawFrameBufferBinding target(_resolved);

    render_state::Capability noDepth(GL_DEPTH_TEST, false);

    TextureUnits units;
    ShaderProgramBinding shaderBinding(shader);
    shaderBinding.setMat4("model", glm::mat4(1.0f));
    shaderBinding.setBool("applyFilter", _parameters.filterOn);
    shaderBinding.setBool("applyMSAA", _parameters.msaaOn);

    const Texture2DMultisample& sceneTexture = _scene.texture();
    TextureDrawBinding sceneTextureBinding(units, sceneTexture);
    shaderBinding.setTexture("screenTexture", sceneTextureBinding);
    shaderBinding.setInt("sampleCount", sceneTexture.samples());

    _canvas.draw();
}

void View::drawLuminance(const ShaderProgram& shader) const {
    {
        DrawFrameBufferBinding target(_luminance);

        render_state::Capability noDepth(GL_DEPTH_TEST, false);

        TextureUnits units;
        ShaderProgramBinding shaderBinding(shader);
        shaderBinding.setMat4("model", glm::mat4(1.0f));
        TextureDrawBinding screenTexture(units, _resolved.texture());
        shaderBinding.setTexture("screenTexture", screenTexture);

        _canvas.draw();
    }
    Texture2DBinding(_luminance.texture()).generateMipmap();
}

void View::drawTonemapped(const ShaderProgram& shader) const {
    DrawFrameBufferBinding target(_output);

    render_state::Capability noDepth(GL_DEPTH_TEST, false);

    TextureUnits units;
    ShaderProgramBinding shaderBinding(shader);
    shaderBinding.setMat4("model", glm::mat4(1.0f));
    shaderBinding.setFloat("gamma", _parameters.gamma);

    TextureDrawBinding screenTexture(units, _resolved.texture());
    shaderBinding.setTexture("screenTexture", screenTexture);
    TextureDrawBinding luminanceLog2(units, _luminance.texture());
    shaderBinding.setTexture("luminanceLog2", luminanceLog2);

    _canvas.draw();
}

const FrameBufferMultisample& View::scene() const { return _scene; }

const FrameBuffer& View::resolved() const { return _resolved; }

const FrameBuffer& View::luminance() const { return _luminance; }

const FrameBuffer& View::output() const { return _output; }

ViewParameters& View::parameters() { return _parameters; }

const ViewParameters& View::parameters() const { return _parameters; }
