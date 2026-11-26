// collect multiple snapshots of an already-running process at a configurable interval
#include "profiler/ProcessMonitor.h"

#include <stdexcept>
#include <thread>

namespace profiler{
    ProcessMonitor::ProcessMonitor(const ProcfsReader& reader) : reader_(reader){

    }

    void ProcessMonitor::monitor(
        pid_t pid,
        std::chrono::milliseconds samplingInterval,
        std::size_t sampleCount,
        const SampleCallBack& callback
    ) const {
        if(samplingInterval.count() <= 0){
            throw std::invalid_argument(
                "Sampling interval must be greater than zero"
            );
        }

        if(sampleCount == 0){
            throw std::invalid_argument(
                "Sample count must be greater than zero"
            );
        }

        if(!callback){
            throw std::invalid_argument(
                "Sample callback cannot be empty"
            );
        }

        for(std::size_t i = 0; i < sampleCount; i++){
            // Read the latest process metrics.
            ProcessMetrics metrics = reader_.readProcessMetrics(pid);

            // Record when this sample was collected.
            metrics.timestamp = std::chrono::steady_clock::now();

            // Deliver the sample to the caller.
            callback(metrics);

            // Wait before collecting the next sample.
            if(i + 1 < sampleCount){
                const auto nextSampleTime = std::chrono::steady_clock::now() + samplingInterval;

                std::this_thread::sleep_until(nextSampleTime);
            }
        }
    }
}