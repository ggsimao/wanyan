#include "render_state/blending.h"

namespace render_state {
    BlendFunc::BlendFunc(GLenum source, GLenum destination) {
        glGetIntegerv(GL_BLEND_SRC_RGB, &_sourceRgb);
        glGetIntegerv(GL_BLEND_DST_RGB, &_destinationRgb);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &_sourceAlpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &_destinationAlpha);
        glBlendFunc(source, destination);
    }

    BlendFunc::~BlendFunc() {
        glBlendFuncSeparate(static_cast<GLenum>(_sourceRgb), static_cast<GLenum>(_destinationRgb),
                            static_cast<GLenum>(_sourceAlpha), static_cast<GLenum>(_destinationAlpha));
    }
}  // namespace render_state
