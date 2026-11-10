#pragma once

#include <string>
#include <vector>

#include "profiler/ProcessResult.h"

namespace profiler
{
    class ProcessLauncher{
        public:
            ProcessResult run(
                const std::string& executable,
                const std::vector<std::string>& arguments
            ) const;
    };
}