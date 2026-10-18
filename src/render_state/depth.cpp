#include "render_state/depth.h"

namespace render_state {
    DepthFunc::DepthFunc(GLenum func) {
        glGetIntegerv(GL_DEPTH_FUNC, &_previous);
        glDepthFunc(func);
    }

    DepthFunc::~DepthFunc() {
        glDepthFunc(static_cast<GLenum>(_previous));
    }
}  // namespace render_state
