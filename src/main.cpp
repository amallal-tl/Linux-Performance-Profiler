#include <iostream>
#include <exception>
#include <vector>
#include <string>

#include "profiler/ProcessLauncher.h"

int main()
{
    try
    {
        profiler::ProcessLauncher launcher;

        //const std::string executable = "/usr/bin/sleep";

        const std::string executable = "/usr/bin/does-not-exist";
        
        const std::vector<std::string> arguments = {
            "sleep",
            "3"
        };

        std::cout << "Profiler started\n";

        const profiler::ProcessResult result =
            launcher.run(executable, arguments);

        std::cout << "\n========== Profiling Result ==========\n";

        std::cout << "Child PID: "
                  << result.pid << '\n';

        std::cout << "Wall-clock time: "
                  << result.wallClockTimeSeconds << " seconds\n";

        std::cout << "User CPU time: "
                  << result.userCpuTimeSeconds << " seconds\n";

        std::cout << "System CPU time: "
                  << result.systemCpuTimeSeconds << " seconds\n";

        std::cout << "Maximum RSS: "
                  << result.maximumResidentMemoryMB << " MB\n";

        if (result.exitedNormally)
        {
            std::cout << "Exit code: "
                      << result.exitCode << '\n';
        }
        else if (result.terminatedBySignal)
        {
            std::cout << "Terminated by signal: "
                      << result.terminatingSignal << '\n';
        }

        std::cout << "======================================\n";
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Profiler error: "
                  << exception.what() << '\n';

        return 1;
    }

    return 0;
}