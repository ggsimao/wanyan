#pragma once

#include <GL/glew.h>

namespace debug {
#ifndef NDEBUG
    inline constexpr bool GL_DEBUG_ENABLED = true;
#else
    inline constexpr bool GL_DEBUG_ENABLED = false;
#endif
    void enableGlDebugOutput();
}
