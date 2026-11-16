#pragma once

#include <random>

namespace random_engine {
    std::mt19937& engine();
    void setSeed(std::uint32_t seed);
}  // namespace random_engine
