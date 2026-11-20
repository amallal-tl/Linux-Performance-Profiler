#pragma once

#include <cstdint>
#include <sys/types.h>
#include <chrono>

namespace profiler
{

struct ProcessMetrics
{
    pid_t pid = -1;
    pid_t parentPid = -1;

    char state = '\0';

    std::uint64_t userCpuTicks = 0;
    std::uint64_t systemCpuTicks = 0;

    std::uint64_t minorPageFaults = 0;
    std::uint64_t majorPageFaults = 0;

    std::uint64_t threadCount = 0;

    std::uint64_t startTimeTicks = 0;

    std::uint64_t virtualMemoryBytes = 0;
    std::int64_t residentPages = 0;

    std::chrono::steady_clock::time_point timestamp;
};

}