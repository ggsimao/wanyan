#include "render_state/culling.h"

namespace render_state {
    CullFace::CullFace(GLenum face) {
        glGetIntegerv(GL_CULL_FACE_MODE, &_previous);
        glCullFace(face);
    }

    CullFace::~CullFace() {
        glCullFace(static_cast<GLenum>(_previous));
    }
}  // namespace render_state
