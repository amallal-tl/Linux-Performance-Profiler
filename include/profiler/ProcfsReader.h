#pragma once

#include "profiler/ProcessMetrics.h"

namespace profiler{
    class ProcfsReader{
        public:
            ProcessMetrics readProcessMetrics(pid_t pid) const;
    };
}