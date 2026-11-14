
#include "profiler/ProcessLauncher.h"

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <system_error>
#include <vector>

#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace profiler
{

ProcessResult ProcessLauncher::run(
    const std::string& executable,
    const std::vector<std::string>& arguments
) const
{
    if (executable.empty() || arguments.empty())
    {
        throw std::invalid_argument(
            "Executable and arguments must not be empty"
        );
    }

    // Create writable argument buffers for execvp().
    std::vector<std::vector<char>> argumentBuffers;

    for (const std::string& argument : arguments)
    {
        argumentBuffers.emplace_back(
            argument.begin(),
            argument.end()
        );

        argumentBuffers.back().push_back('\0');
    }

    // Create the char* argument array.
    std::vector<char*> argv;

    for (std::vector<char>& buffer : argumentBuffers)
    {
        argv.push_back(buffer.data());
    }

    argv.push_back(nullptr);

    // Create a pipe with close-on-exec enabled.
    int pipeFd[2];

    if (pipe2(pipeFd, O_CLOEXEC) == -1)
    {
        throw std::system_error(
            errno,
            std::generic_category(),
            "pipe2 failed"
        );
    }

    const auto startTime = std::chrono::steady_clock::now();

    // Create the child process.
    const pid_t childPid = fork();

    if (childPid == -1)//child process creation failed
    {
        const int error = errno;

        close(pipeFd[0]);//Close read
        close(pipeFd[1]);//Close write

        throw std::system_error(
            error,
            std::generic_category(),
            "fork failed"
        );
    }

    if (childPid == 0)//Child creation success
    {
        // Child process.

        close(pipeFd[0]);//Close read

        execvp(executable.c_str(), argv.data());

        // Reaching here means execvp failed.
        const int execError = errno;

        // Send the error number to the parent.
        const char* errorData =
            reinterpret_cast<const char*>(&execError);

        std::size_t bytesWritten = 0;

        while (bytesWritten < sizeof(execError))
        {
            const ssize_t count = write(
                pipeFd[1],
                errorData + bytesWritten,
                sizeof(execError) - bytesWritten
            );

            if (count > 0)
            {
                bytesWritten += static_cast<std::size_t>(count);
            }
            else if (count == -1 && errno == EINTR)
            {
                continue;
            }
            else
            {
                break;
            }
        }

        close(pipeFd[1]);//Close write

        _exit(127);
    }

    // Parent process.

    close(pipeFd[1]);//Close write

    int execError = 0;
    std::size_t bytesRead = 0;

    // Read the exec failure information, if any.
    while (bytesRead < sizeof(execError))
    {
        const ssize_t count = read(
            pipeFd[0],
            reinterpret_cast<char*>(&execError) + bytesRead,
            sizeof(execError) - bytesRead
        );

        if (count > 0)
        {
            bytesRead += static_cast<std::size_t>(count);
        }
        else if (count == 0)
        {
            break;
        }
        else if (errno == EINTR)
        {
            continue;
        }
        else
        {
            const int error = errno;
            close(pipeFd[0]);

            // Reap the child before reporting the read failure.
            int ignoredStatus = 0;
            struct rusage ignoredUsage {};

            while (wait4(
                childPid,
                &ignoredStatus,
                0,
                &ignoredUsage
            ) == -1 && errno == EINTR)
            {
            }

            throw std::system_error(
                error,
                std::generic_category(),
                "Reading exec error pipe failed"
            );
        }
    }

    close(pipeFd[0]);

    // Collect the child's termination status and resource usage.
    int status = 0;
    struct rusage usage {};

    pid_t waitResult;

    do
    {
        waitResult = wait4(childPid, &status, 0, &usage);
    }
    while (waitResult == -1 && errno == EINTR);

    if (waitResult == -1)
    {
        throw std::system_error(
            errno,
            std::generic_category(),
            "wait4 failed"
        );
    }

    // Check whether execvp actually succeeded.
    if (bytesRead == sizeof(execError))
    {
        throw std::system_error(
            execError,
            std::generic_category(),
            "execvp failed"
        );
    }

    if (bytesRead != 0)
    {
        throw std::runtime_error(
            "Incomplete error information received from child"
        );
    }

    const auto endTime = std::chrono::steady_clock::now();

    const std::chrono::duration<double> elapsedTime =
        endTime - startTime;

    ProcessResult result;

    result.pid = childPid;
    result.wallClockTimeSeconds = elapsedTime.count();

    result.userCpuTimeSeconds =
        usage.ru_utime.tv_sec +
        usage.ru_utime.tv_usec / 1000000.0;

    result.systemCpuTimeSeconds =
        usage.ru_stime.tv_sec +
        usage.ru_stime.tv_usec / 1000000.0;

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
