#pragma once

#include <GL/glew.h>

namespace render_state {
    // The blend factors (glBlendFunc) for a scope, e.g. GL_SRC_ALPHA and
    // GL_ONE_MINUS_SRC_ALPHA for ordinary transparency. Restores the previous
    // factors, including separate alpha factors if some code set them. Doesn't
    // enable blending: see Capability.
    class BlendFunc {
    public:
        BlendFunc(GLenum source, GLenum destination);
        ~BlendFunc();
        BlendFunc(const BlendFunc&) = delete;
        BlendFunc& operator=(const BlendFunc&) = delete;

    private:
        GLint _sourceRgb = GL_ONE;
        GLint _destinationRgb = GL_ZERO;
        GLint _sourceAlpha = GL_ONE;
        GLint _destinationAlpha = GL_ZERO;
    };
}  // namespace render_state
