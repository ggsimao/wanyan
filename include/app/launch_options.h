#pragma once

#include "app/platform.h"
#include <cstdint>

struct LaunchOptions {
    std::uint32_t seed = 0;
    PlatformOptions platformOptions;
};
