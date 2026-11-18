#pragma once

#include <cstdint>

// Render layers: each MeshRenderer is on one layer (a single bit), and each
// Camera draws only the layers in its mask. Shadow passes ignore layers.
namespace layers {
    constexpr std::uint32_t NONE = 0;
    constexpr std::uint32_t DEFAULT = 1u << 0;
    constexpr std::uint32_t PLAYER_1 = 1u << 1;  // player 1's own model
    constexpr std::uint32_t PLAYER_2 = 1u << 2;  // player 2's own model
    constexpr std::uint32_t ALL = ~0u;
}  // namespace layers
