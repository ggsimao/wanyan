#include "render_state/stencil.h"

namespace render_state {
    StencilFunc::StencilFunc(GLenum func, GLint ref, GLuint mask) {
        glGetIntegerv(GL_STENCIL_FUNC, &_func);
        glGetIntegerv(GL_STENCIL_REF, &_ref);
        glGetIntegerv(GL_STENCIL_VALUE_MASK, &_mask);
        glStencilFunc(func, ref, mask);
    }

    StencilFunc::~StencilFunc() {
        glStencilFunc(static_cast<GLenum>(_func), _ref, static_cast<GLuint>(_mask));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    StencilOp::StencilOp(GLenum stencilFail, GLenum depthFail, GLenum pass) {
        glGetIntegerv(GL_STENCIL_FAIL, &_stencilFail);
        glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &_depthFail);
        glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &_pass);
        glStencilOp(stencilFail, depthFail, pass);
    }

    StencilOp::~StencilOp() {
        glStencilOp(static_cast<GLenum>(_stencilFail), static_cast<GLenum>(_depthFail),
                    static_cast<GLenum>(_pass));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    StencilMask::StencilMask(GLuint mask) {
        glGetIntegerv(GL_STENCIL_WRITEMASK, &_previous);
        glStencilMask(mask);
    }

    StencilMask::~StencilMask() {
        glStencilMask(static_cast<GLuint>(_previous));
    }
}  // namespace render_state
