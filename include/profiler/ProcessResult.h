#pragma once

#include <sys/types.h>

namespace profiler
{
    struct ProcessResult
    {
        pid_t pid = -1;
        double wallClockTimeSeconds = 0.0;
        double userCpuTimeSeconds = 0.0;
        double systemCpuTimeSeconds = 0.0;
        double maximumResidentMemoryMB = 0.0;
        int exitCode = -1;
        bool exitedNormally = false;
        bool terminatedBySignal = false;
        int terminatingSignal = 0;
    };
}