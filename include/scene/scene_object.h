#pragma once

#include "data/buffer.h"
#include "drawables/drawable.h"
#include "scene/instance.h"
#include "spatial.h"

#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

class ShaderProgramBinding;
class TextureUnits;

// Something placed in the scene: a drawable, drawn once per instance, each
// instance with its own transform. Starts with one instance at the origin.
//
// The object's own model matrix (from Spatial) goes to the uniform buffer;
// each instance's matrices go to the instance buffer.
class SceneObject : public Spatial<SceneObject> {
public:
    // Only takes part in overload resolution for Drawable types, so it can't
    // capture SceneObject's own moves or the private constructor's unique_ptr.
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<Drawable, T>>>
    explicit SceneObject(T drawable)
        : SceneObject(std::make_unique<T>(std::move(drawable))) {}

    // Spatial
    const glm::mat4& model() const;
    void setModel(const glm::mat4& model);

    // Instances
    void addInstances(std::size_t count = 1);
    std::size_t instanceCount() const;
    const Instance& instance(std::size_t index) const;
    Instance& instance(std::size_t index);  // marks the instances for re-upload
    Instance& lastInstance();               // Rust: get_instance_mut(-1)

    // Outline
    const glm::vec3& outlineColor() const;
    void setOutlineColor(const glm::vec3& color);
    void enableOutline(bool enable);
    bool hasOutline() const;

    const Drawable& drawable() const;

    // Draws every instance. Re-uploads the instances first if any changed.
    void draw(const ShaderProgramBinding& shader, TextureUnits& units) const;
    // Draws the outline around the stencil written by draw(); the caller sets
    // up the outline shader and the scaled model matrix.
    void drawOutline(const ShaderProgramBinding& shader, TextureUnits& units) const;

private:
    explicit SceneObject(std::unique_ptr<Drawable> drawable);
    void uploadInstances() const;

    std::unique_ptr<Drawable> _drawable;
    std::vector<Instance> _instances{Instance()};
    ArrayBuffer _instanceBuffer;
    glm::mat4 _model{1.0f};
    glm::vec3 _outlineColor{0.0f};
    bool _outlined = false;
    // Instances are uploaded lazily, on the first draw after a change. That's
    // a cache of state the object logically already has, so draw() stays const.
    mutable bool _instancesDirty = true;
};
