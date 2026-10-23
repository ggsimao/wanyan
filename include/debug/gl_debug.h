#pragma once

#include <GL/glew.h>

namespace debug {
#ifndef NDEBUG
    inline constexpr bool DEBUG_ON = true;
#else
    inline constexpr bool DEBUG_ON = false;
#endif
    void enableGlDebugOutput();
}
