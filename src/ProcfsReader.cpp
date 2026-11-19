#include "profiler/ProcfsReader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace profiler{
    ProcessMetrics ProcfsReader::readProcessMetrics(pid_t pid) const
    {
        // Step 1: Construct the procfs file path.
        const std::string filePath = "/proc" + std::to_string(pid) + "/stat";

        //Step 2: Open the file
        std::ifstream file(filePath);

        if(!file.is_open()){
            throw std::runtime_error("Unable to open process stat file : " + filePath);
        }

        // Step 3: Read the complete line.
        std::string line;
        if(!std::getline(file, line)){
            throw std::runtime_error(
                "Unable to read process stat file " + filePath
            );
        }

        // Step 4: Find the process name boundaries.

        const std::size_t firstSpace = line.find(' ');
        const std::size_t closingParen = line.rfind(')');

        if (firstSpace == std::string::npos ||
            closingParen == std::string::npos ||
            closingParen <= firstSpace ||
            closingParen + 2 > line.size())
        {
            throw std::runtime_error(
                "Invalid format in process stat file"
            );
        }

        // Step 5: Extract the PID.

        const pid_t actualPid = static_cast<pid_t>(
            std::stol(line.substr(0, firstSpace))
        );

        if (actualPid != pid)
        {
            throw std::runtime_error(
                "PID mismatch while reading process stat"
            );
        }


        // Step 6: Parse the fields after the process name.

        const std::string remainingFields =
            line.substr(closingParen + 2);

        std::istringstream stream(remainingFields);


        // Linux /proc/PID/stat fields, starting from field 3.

        char state = '\0';

        long parentPid = 0;
        long processGroup = 0;
        long sessionId = 0;
        long terminalNumber = 0;
        long foregroundProcessGroup = 0;

        unsigned long flags = 0;

        unsigned long long minorFaults = 0;
        unsigned long long childMinorFaults = 0;

        unsigned long long majorFaults = 0;
        unsigned long long childMajorFaults = 0;

        unsigned long long userCpuTicks = 0;
        unsigned long long systemCpuTicks = 0;

        long long childUserCpuTicks = 0;
        long long childSystemCpuTicks = 0;

        long priority = 0;
        long niceValue = 0;
        long threadCount = 0;

        long long intervalTimer = 0;

        unsigned long long startTimeTicks = 0;
        unsigned long long virtualMemoryBytes = 0;

        long long residentPages = 0;


        // Extract fields in their original Linux order.

        if (!(stream
            >> state
            >> parentPid
            >> processGroup
            >> sessionId
            >> terminalNumber
            >> foregroundProcessGroup
            >> flags
            >> minorFaults
            >> childMinorFaults
            >> majorFaults
            >> childMajorFaults
            >> userCpuTicks
            >> systemCpuTicks
            >> childUserCpuTicks
            >> childSystemCpuTicks
            >> priority
            >> niceValue
            >> threadCount
            >> intervalTimer
            >> startTimeTicks
            >> virtualMemoryBytes
            >> residentPages))
        {
            throw std::runtime_error(
                "Failed to parse process stat fields"
            );
        }


        // Step 7: Populate our domain structure.

        ProcessMetrics metrics;

        metrics.pid = actualPid;
        metrics.parentPid = static_cast<pid_t>(parentPid);

        metrics.state = state;

        metrics.userCpuTicks = userCpuTicks;
        metrics.systemCpuTicks = systemCpuTicks;

        metrics.minorPageFaults = minorFaults;
        metrics.majorPageFaults = majorFaults;

        metrics.threadCount = static_cast<std::uint64_t>(threadCount);

        metrics.startTimeTicks = startTimeTicks;

        metrics.virtualMemoryBytes = virtualMemoryBytes;
        metrics.residentPages = residentPages;

        return metrics;
    }
}