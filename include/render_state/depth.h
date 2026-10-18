#pragma once

#include <GL/glew.h>

namespace render_state {
    // Sets the depth comparison (GL_LESS, GL_LEQUAL, ...) for a scope, then
    // restores the previous one. Doesn't enable depth testing: see Capability.
    class DepthFunc {
    public:
        explicit DepthFunc(GLenum func);
        ~DepthFunc();
        DepthFunc(const DepthFunc&) = delete;
        DepthFunc& operator=(const DepthFunc&) = delete;

    private:
        GLint _previous = GL_LESS;
    };
}  // namespace render_state
