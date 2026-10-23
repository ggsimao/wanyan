#include "debug/frame_stats.h"

#include <cstddef>
#include <iostream>

namespace {
    constexpr const char* SECTION_NAMES[] = {"Control update", "Instance move", "Draw"};

    double milliseconds(std::chrono::steady_clock::duration duration) {
        return std::chrono::duration<double, std::milli>(duration).count();
    }
}  // namespace

debug::FrameStats::Timer::Timer(FrameStats& stats, Section section)
    : _stats(stats), _section(section), _start(Clock::now()), _scope(stats._gpu[stats._slot][section].query) {
    _stats._gpu[_stats._slot][section].pending = true;
}

debug::FrameStats::Timer::~Timer() {
    _stats._cpuTotals[_section] += Clock::now() - _start;
}

debug::FrameStats::FrameStats() : _periodStart(Clock::now()) {}

debug::FrameStats::Timer debug::FrameStats::time(Section section) {
    return Timer(*this, section);
}

void debug::FrameStats::endFrame(bool report) {
    ++_frames;
    _slot = (_slot + 1) % FRAMES_IN_FLIGHT;
    for (std::size_t i = 0; i < SECTION_COUNT; ++i) {
        if (_gpu[_slot][i].pending && _gpu[_slot][i].query.available()) {
            _gpuTotals[i] += std::chrono::nanoseconds(_gpu[_slot][i].query.result());
            ++_samples[i];
        }
        _gpu[_slot][i].pending = false;
    }

    const Clock::duration elapsed = Clock::now() - _periodStart;
    if (elapsed < std::chrono::seconds(1)) {
        return;
    }

    if (report) {
        const auto frames = static_cast<double>(_frames);
        for (std::size_t i = 0; i < SECTION_COUNT; ++i) {
            std::cout << SECTION_NAMES[i] << ": CPU " << milliseconds(_cpuTotals[i]) / frames << " ms, ";
            if (_samples[i] == 0)
                std::cout << "GPU no data\n";
            else
                std::cout << "GPU " << milliseconds(_gpuTotals[i]) / _samples[i] << " ms\n";
        }
        std::cout << "FPS: " << frames / (milliseconds(elapsed) / 1000.0) << '\n'
                  << "----------------------------------------\n";
    }

    _cpuTotals = {};
    _gpuTotals = {};
    _samples = {};
    _frames = 0;
    _periodStart = Clock::now();
}
