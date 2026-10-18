#pragma once

#include <array>
#include <chrono>
#include <cstddef>

// Average time spent per frame in each section, plus frames per second,
// reported once per second. Off unless enabled (F3), but always measuring:
// a few clock reads per frame.
//
//     { auto timer = stats.time(FrameStats::DRAW); /* draw */ }
//     stats.endFrame(program.showStats());
class FrameStats {
public:
    enum Section { UPDATE, INSTANCES, DRAW, SECTION_COUNT };

    // Adds the time from its construction to its destruction to a section.
    class Timer {
    public:
        Timer(const Timer&) = delete;
        Timer& operator=(const Timer&) = delete;
        Timer(Timer&&) = delete;
        Timer& operator=(Timer&&) = delete;
        ~Timer();

    private:
        friend class FrameStats;
        Timer(FrameStats& stats, Section section);

        FrameStats& _stats;
        Section _section;
        std::chrono::steady_clock::time_point _start;
    };

    FrameStats();

    [[nodiscard]] Timer time(Section section);

    // Call once per frame. Every second, prints the averages over that second
    // if `report` is set, then starts a new second.
    void endFrame(bool report);

private:
    using Clock = std::chrono::steady_clock;

    std::array<Clock::duration, SECTION_COUNT> _totals{};
    std::size_t _frames = 0;
    Clock::time_point _periodStart;
};
