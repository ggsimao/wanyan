#include "scene/scene_object.h"

#include "render_state/capability.h"
#include "render_state/stencil.h"
#include "textures/draw_binding.h"
#include "scoped_binding.h"
#include "shaders/shader_program.h"

#include <cassert>

namespace {
    // Uploads to an array buffer outside of any VAO. The VAO-bound
    // ArrayBufferBinding is for attribute setup; this only replaces data.
    class InstanceBufferBinding : ScopedBinding<ArrayBuffer> {
    public:
        explicit InstanceBufferBinding(const ArrayBuffer& buffer) : ScopedBinding(buffer) {}

        void upload(const std::vector<Instance>& instances) {
            assertActive();
            // Rewritten whenever instances move, often every frame.
            glBufferData(GL_ARRAY_BUFFER,
                        static_cast<GLsizeiptr>(instances.size() * sizeof(Instance)),
                        instances.data(), GL_DYNAMIC_DRAW);
        }
    };
}  // namespace

SceneObject::SceneObject(std::unique_ptr<Drawable> drawable) : _drawable(std::move(drawable)) {
    uploadInstances();
    _drawable->setupInstanceAttributes(_instanceBuffer);
}

const glm::mat4& SceneObject::model() const {
    return _model;
}

void SceneObject::setModel(const glm::mat4& model) {
    _model = model;
}

void SceneObject::addInstances(std::size_t count) {
    _instances.resize(_instances.size() + count);
    _instancesDirty = true;
}

std::size_t SceneObject::instanceCount() const {
    return _instances.size();
}

const Instance& SceneObject::instance(std::size_t index) const {
    assert(index < _instances.size() && "Instance index out of range");
    return _instances[index];
}

Instance& SceneObject::instance(std::size_t index) {
    assert(index < _instances.size() && "Instance index out of range");
    _instancesDirty = true;
    return _instances[index];
}

Instance& SceneObject::lastInstance() {
    _instancesDirty = true;
    return _instances.back();
}

const glm::vec3& SceneObject::outlineColor() const {
    return _outlineColor;
}

void SceneObject::setOutlineColor(const glm::vec3& color) {
    _outlineColor = color;
}

void SceneObject::enableOutline(bool enable) {
    _outlined = enable;
}

bool SceneObject::hasOutline() const {
    return _outlined;
}

const Drawable& SceneObject::drawable() const {
    return *_drawable;
}

void SceneObject::draw(const ShaderProgramBinding& shader, TextureUnits& units) const {
    if (_instancesDirty) {
        uploadInstances();
    }
    _drawable->drawInstanced(shader, units, static_cast<GLsizei>(_instances.size()));
}

void SceneObject::drawOutline(const ShaderProgramBinding& shader, TextureUnits& units) const {
    // Only where the object itself didn't write to the stencil buffer, on top
    // of everything else. Each guard restores whatever was set before.
    render_state::StencilFunc outsideObject(GL_NOTEQUAL, 1, 0xFF);
    render_state::StencilMask freezeStencil(0x00);
    render_state::Capability noDepth(GL_DEPTH_TEST, false);

    shader.setVec3("outlineColor", _outlineColor);
    _drawable->draw(shader, units);
}

void SceneObject::uploadInstances() const {
    InstanceBufferBinding buffer(_instanceBuffer);
    buffer.upload(_instances);
    _instancesDirty = false;
}
