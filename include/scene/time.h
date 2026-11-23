#pragma once

#include <cmath>
#include <cstdint>

struct Time {
  public:
    Time(std::uint32_t dayDuration, std::uint32_t yearDuration)
        : dayDuration(dayDuration), yearDuration(yearDuration) {}
    void advance(float dt) {
        days = std::fmod(days + (dt / dayDuration),
                         static_cast<float>(yearDuration));
    }
    float days = 0.0f;
    std::uint32_t dayDuration, yearDuration;
};
