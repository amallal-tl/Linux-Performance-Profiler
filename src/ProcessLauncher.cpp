#include "profiler/ProcessLauncher.h"

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <stdexcept>
#include <vector>

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace Profiler
{
    ProcessResult ProcessLauncher::run(
        const std::string& executable,
        const std::vector<std::string& arguments
    ) const 
    {
        if(executable.empty() || argument.empty()){
            throw std::invalid_argument(
                "Executables and arguments must not be empty"
            );
        }

        // Build the argument array required by execvp().
        std::vector<char*> argv;
        argv.reserve(arguments.size() + 1);

        for(const std::string& argument : arguments)
        {
            argv.push_back(const_cast<char*>(arguments.c_str()));
        }

        // execvp() expects the argument array to end with nullptr.
        argv.push_back(nullptr);

        // Start measuring elapsed time.
        const auto startTime = std::chrono::steady_clock::now();

        const pid_t childPid = fork();

        if(childPid < 0){
            throw std::runtime_error("fork failed");
        }

        if(childPid == 0){
            // Child process: replace this process with the target program.
            execvp(executable.c_str(), argv.data());

            // Reaching here means execvp() failed.
            perror("execvp failed");
            _exit(127);
        }

        // Parent process: wait for the child and collect resource usage.
        int status = 0;
        struct rusage usage{};

        pid_t waitResult;

        do{
            waitResult = wait4(childPid, &status, 0, &usage);
        } while(waitResult == -1 && errorno == EINTR);

        if (waitResult == -1)
        {
            throw std::runtime_error("wait4() failed");
        }

        const auto endTime = std::chrono::steady_clock::now();

        const std::chrono::duration<double> elapsedTime =
            endTime - startTime;

        // Convert the collected measurements into our result structure.
        ProcessResult result;

        result.pid = childPid;
        result.wallClockTimeSeconds = elapsedTime.count();

        result.userCpuTimeSeconds =
            usage.ru_utime.tv_sec +
            usage.ru_utime.tv_usec / 1000000.0;

        result.systemCpuTimeSeconds =
            usage.ru_stime.tv_sec +
            usage.ru_stime.tv_usec / 1000000.0;

        // Linux reports ru_maxrss in KiB.
        result.maximumResidentMemoryMB =
            usage.ru_maxrss / 1024.0;

        if (WIFEXITED(status))
        {
            result.exitedNormally = true;
            result.exitCode = WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status))
        {
            result.terminatedBySignal = true;
            result.terminatingSignal = WTERMSIG(status);
        }

        return result;

    }
}