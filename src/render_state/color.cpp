#include "render_state/color.h"

namespace render_state {
    ColorMask::ColorMask(bool writeColor) {
        glGetBooleanv(GL_COLOR_WRITEMASK, _previous.data());
        const GLboolean write = writeColor ? GL_TRUE : GL_FALSE;
        glColorMask(write, write, write, write);
    }

    ColorMask::~ColorMask() {
        glColorMask(_previous[0], _previous[1], _previous[2], _previous[3]);
    }
}  // namespace render_state
