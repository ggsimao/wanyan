#pragma once

#include "data/query.h"
#include <array>
#include <chrono>
#include <cstddef>

namespace debug {
    static constexpr std::size_t FRAMES_IN_FLIGHT = 3;

    struct GpuSample {
        TimeElapsedQuery query;
        bool pending = false;  // begun, result not yet collected
    };
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
            TimeElapsedQueryScope _scope;
        };

        FrameStats();

        [[nodiscard]] Timer time(Section section);

        // Call once per frame. Every second, prints the averages over that second
        // if `report` is set, then starts a new second.
        void endFrame(bool report);

    private:
        using Clock = std::chrono::steady_clock;

        std::array<Clock::duration, SECTION_COUNT> _cpuTotals{}, _gpuTotals{};
        std::array<std::size_t, SECTION_COUNT> _samples{};

        std::array<std::array<GpuSample, SECTION_COUNT>, FRAMES_IN_FLIGHT> _gpu;
        std::size_t _slot = 0;  // the frame slot in use, advanced in endFrame
        std::size_t _frames = 0;
        Clock::time_point _periodStart;
    };
}
