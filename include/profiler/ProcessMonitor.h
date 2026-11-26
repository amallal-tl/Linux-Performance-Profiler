// collect multiple snapshots of an already-running process at a configurable interval

#pragma once

#include <chrono>
#include <cstddef>
#include <functional>

#include "profiler/ProcfsReader.h"

namespace profiler{
    class ProcessMonitor{
        public:
            using SampleCallBack = std::function<void(const ProcessMetrics&)>;

            explicit ProcessMonitor(const ProcfsReader& reader);

            void monitor(
                pid_t pid,
                std::chrono::milliseconds samplingInterval,
                std::size_t sampleCount,
                const SampleCallBack& callback
            ) const;

        private:
            const ProcfsReader& reader_;
    };
}