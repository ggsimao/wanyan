#pragma once

#include "data/buffer.h"

#include <GL/glew.h>

class ShaderProgramBinding;
class TextureUnits;

// Anything a SceneObject can draw: meshes, models, skyboxes, canvases.
class Drawable {
public:
    virtual ~Drawable() = default;

    virtual void draw(const ShaderProgramBinding& shader, TextureUnits& units) const = 0;
    virtual void drawPlain(const ShaderProgramBinding& shader) const = 0;

    // Default: things that don't support instancing draw a single copy.
    virtual void drawInstanced(const ShaderProgramBinding& shader, TextureUnits& units,
                               GLsizei /*instances*/) const {
        draw(shader, units);
    }
    virtual void drawPlainInstanced(const ShaderProgramBinding& shader, GLsizei /*instances*/) const {
        drawPlain(shader);
    }

    // Default: nothing to set up for things that don't support instancing.
    virtual void setupInstanceAttributes(const ArrayBuffer& /*instanceBuffer*/) const {}

protected:
    // Protected so a Drawable can't be copied or moved through a base
    // reference (which would slice it), while derived classes keep their
    // implicit copy and move operations.
    Drawable() = default;
    Drawable(const Drawable&) = default;
    Drawable& operator=(const Drawable&) = default;
    Drawable(Drawable&&) = default;
    Drawable& operator=(Drawable&&) = default;
};
