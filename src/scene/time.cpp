#include "scene/time.h"
#include <cmath>

Time::Time(std::uint32_t dayDuration, std::uint32_t yearDuration)
    : _dayDuration(dayDuration), _yearDuration(yearDuration) {}

float Time::getDays() const { return _days; }

std::uint32_t Time::getDayDuration() const { return _dayDuration; }

std::uint32_t Time::getYearDuration() const { return _yearDuration; }

void Time::advance(float dt) {
    _days = std::fmod(_days + (dt / _dayDuration),
                      static_cast<float>(_yearDuration));
}

void Time::setDays(float days) { _days = days; }

void Time::setDayDuration(std::uint32_t duration) { _dayDuration = duration; }

void Time::setYearDuration(std::uint32_t duration) { _yearDuration = duration; }
