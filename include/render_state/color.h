#pragma once

#include <GL/glew.h>

#include <array>

namespace render_state {
    // Which color channels draws may write (glColorMask), for a scope. All
    // false lets a pass touch only depth or stencil.
    class ColorMask {
      public:
        explicit ColorMask(bool writeColor);
        ~ColorMask();
        ColorMask(const ColorMask&) = delete;
        ColorMask& operator=(const ColorMask&) = delete;

      private:
        std::array<GLboolean, 4> _previous{GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE};
    };
}  // namespace render_state
