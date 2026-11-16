#pragma once

#include <cstdint>

class InputState;
class Scene;

struct FrameContext {
    std::uint32_t seed;
    float dt = 0;
    std::uint64_t frame = 0;
    Scene& scene;
    const InputState& input;
};
