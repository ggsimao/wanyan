#pragma once

#include <compare>
#include <cstdint>
#include <limits>

struct EntityHandle {
    static constexpr std::uint32_t NO_INDEX =
        std::numeric_limits<std::uint32_t>::max();

    std::uint32_t index = NO_INDEX;
    std::uint32_t generation = 0;

    bool isNull() const { return index == NO_INDEX; }
    bool operator==(const EntityHandle&) const = default;
    std::strong_ordering operator<=>(const EntityHandle&) const = default;
};
