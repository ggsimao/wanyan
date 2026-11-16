#include "random.h"
#include <cassert>

namespace {
    static bool seeded = false;
}

std::mt19937& random_engine::engine() {
    assert(seeded && "random_engine::setSeed must be called before "
                     "generating random values");
    static std::mt19937 instance;
    return instance;
}
void random_engine::setSeed(std::uint32_t seed) {
    seeded = true;
    engine().seed(seed);
}
