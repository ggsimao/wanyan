#include "debug/frame_stats.h"

#include <iostream>

namespace {
    constexpr const char* SECTION_NAMES[] = {"Control update", "Instance move", "Draw"};

    double milliseconds(std::chrono::steady_clock::duration duration) {
        return std::chrono::duration<double, std::milli>(duration).count();
    }
}  // namespace

FrameStats::Timer::Timer(FrameStats& stats, Section section)
    : _stats(stats), _section(section), _start(Clock::now()) {}

FrameStats::Timer::~Timer() {
    _stats._totals[_section] += Clock::now() - _start;
}

FrameStats::FrameStats() : _periodStart(Clock::now()) {}

FrameStats::Timer FrameStats::time(Section section) {
    return Timer(*this, section);
}

void FrameStats::endFrame(bool report) {
    ++_frames;
    const Clock::duration elapsed = Clock::now() - _periodStart;
    if (elapsed < std::chrono::seconds(1)) {
        return;
    }

    if (report) {
        const auto frames = static_cast<double>(_frames);
        for (std::size_t i = 0; i < SECTION_COUNT; ++i) {
            std::cout << SECTION_NAMES[i] << ": " << milliseconds(_totals[i]) / frames << " ms\n";
        }
        std::cout << "FPS: " << frames / (milliseconds(elapsed) / 1000.0) << '\n'
                  << "----------------------------------------\n";
    }

    _totals = {};
    _frames = 0;
    _periodStart = Clock::now();
}
