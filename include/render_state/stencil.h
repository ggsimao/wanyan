#pragma once

#include <GL/glew.h>

namespace render_state {
    // The stencil test's comparison (glStencilFunc) for a scope. Doesn't enable
    // stencil testing: see Capability.
    class StencilFunc {
    public:
        StencilFunc(GLenum func, GLint ref, GLuint mask);
        ~StencilFunc();
        StencilFunc(const StencilFunc&) = delete;
        StencilFunc& operator=(const StencilFunc&) = delete;

    private:
        GLint _func = GL_ALWAYS;
        GLint _ref = 0;
        GLint _mask = -1;  // all bits; GL reports 0xFFFFFFFF as -1 through a GLint
    };

    // What happens to the stencil buffer when the stencil test fails, the depth
    // test fails, or both pass (glStencilOp), for a scope.
    class StencilOp {
    public:
        StencilOp(GLenum stencilFail, GLenum depthFail, GLenum pass);
        ~StencilOp();
        StencilOp(const StencilOp&) = delete;
        StencilOp& operator=(const StencilOp&) = delete;

    private:
        GLint _stencilFail = GL_KEEP;
        GLint _depthFail = GL_KEEP;
        GLint _pass = GL_KEEP;
    };

    // Which stencil bits draws may write (glStencilMask), for a scope.
    // 0x00 freezes the stencil buffer.
    class StencilMask {
    public:
        explicit StencilMask(GLuint mask);
        ~StencilMask();
        StencilMask(const StencilMask&) = delete;
        StencilMask& operator=(const StencilMask&) = delete;

    private:
        GLint _previous = -1;
    };
}  // namespace render_state
