#pragma once

#include <cstdint>

struct Time {
  public:
    Time(std::uint32_t dayDuration, std::uint32_t yearDuration);
    float getDays() const;
    std::uint32_t getDayDuration() const;
    std::uint32_t getYearDuration() const;
    void advance(float dt);
    void setDays(float days);
    void setDayDuration(std::uint32_t duration);
    void setYearDuration(std::uint32_t duration);

  private:
    float _days = 0.0f;
    std::uint32_t _dayDuration, _yearDuration;
};
