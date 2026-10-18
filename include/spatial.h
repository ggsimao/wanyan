#pragma once

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

// Transform operations for anything with a model matrix.
//
// A CRTP base rather than a virtual interface: the derived class passes itself
// as Derived, and provides
//
//     const glm::mat4& model() const;
//     void setModel(const glm::mat4& model);
//
// Spatial calls those directly, without a vtable. That matters for Instance,
// which is uploaded to the GPU byte for byte: a vtable pointer would add 8
// bytes to its layout. It's also what the Rust version did, since functions
// took `impl Spatial` (static dispatch), not `dyn Spatial`.
template <typename Derived>
class Spatial {
public:
    // For transforming normals: the inverse transpose of the model's
    // upper-left 3x3, which stays correct under non-uniform scaling.
    glm::mat3 normalMatrix() const {
        return glm::inverseTranspose(glm::mat3(self().model()));
    }

    // Rotates about the object's own position, not the origin.
    void rotate(float angle, const glm::vec3& axis) {
        applyRotation(glm::rotate(glm::mat4(1.0f), angle, axis));
    }

    void applyRotation(const glm::mat4& rotation) {
        applyAboutPosition(rotation);
    }

    // Scales about the object's own position, not the origin.
    void scale(const glm::vec3& factors) {
        applyScaling(glm::scale(glm::mat4(1.0f), factors));
    }

    void applyScaling(const glm::mat4& scaling) {
        applyAboutPosition(scaling);
    }

    void translate(const glm::vec3& offset) {
        glm::mat4 model = self().model();
        model[3] += glm::vec4(offset, 0.0f);
        self().setModel(model);
    }

protected:
    // Only usable as a base; never destroyed or copied through Spatial itself.
    Spatial() = default;
    ~Spatial() = default;
    Spatial(const Spatial&) = default;
    Spatial& operator=(const Spatial&) = default;
    Spatial(Spatial&&) = default;
    Spatial& operator=(Spatial&&) = default;

private:
    // Moves the object to the origin, applies the transform, and moves it back,
    // so rotations and scalings don't swing it around the world origin.
    void applyAboutPosition(const glm::mat4& transform) {
        glm::mat4 model = self().model();
        const glm::vec4 position = model[3];
        model[3] = glm::vec4(0.0f, 0.0f, 0.0f, position.w);
        model = transform * model;
        model[3] = position;
        self().setModel(model);
    }

    Derived& self() { return static_cast<Derived&>(*this); }
    const Derived& self() const { return static_cast<const Derived&>(*this); }
};
