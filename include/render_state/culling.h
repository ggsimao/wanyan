#pragma once

#include <GL/glew.h>

namespace render_state {
    // Sets which faces culling removes (GL_BACK, GL_FRONT) for a scope, then
    // restores the previous setting. Doesn't enable culling: see Capability.
    class CullFace {
    public:
        explicit CullFace(GLenum face);
        ~CullFace();
        CullFace(const CullFace&) = delete;
        CullFace& operator=(const CullFace&) = delete;

    private:
        GLint _previous = GL_BACK;
    };
}  // namespace render_state
