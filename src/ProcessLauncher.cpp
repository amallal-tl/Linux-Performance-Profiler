
#include "profiler/ProcessLauncher.h"
#include "profiler/FileDescriptor.h"

#include <cerrno>
#include <chrono>
#include <stdexcept>
#include <system_error>
#include <vector>
#include <cstddef>

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

    // Prepare writable argument buffers before fork().
    std::vector<std::vector<char>> argumentBuffers;

    for (const std::string& argument : arguments)
    {
        argumentBuffers.emplace_back(
            argument.begin(),
            argument.end()
        );

        argumentBuffers.back().push_back('\0');
    }

    // Build the argv array required by execvp().
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
        const int error = errno;

        throw std::system_error(
            error,
            std::generic_category(),
            "pipe2 failed"
        );
    }

    // Transfer ownership of both descriptors to RAII objects.
    FileDescriptor readEnd(pipeFd[0]);
    FileDescriptor writeEnd(pipeFd[1]);

    const auto startTime = std::chrono::steady_clock::now();

    // Create the child process.
    const pid_t childPid = fork();

    if (childPid == -1)
    {
        const int error = errno;

        // No manual close required.
        // Both FileDescriptor destructors run during exception unwinding.

        throw std::system_error(
            error,
            std::generic_category(),
            "fork failed"
        );
    }

    if (childPid == 0)
    {
        // Child process.

        // Child only needs the write end.
        readEnd.reset();

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
                writeEnd.get(),
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

        // Close the child's write descriptor.
        writeEnd.reset();

        _exit(127);
    }

    // Parent process.

    // Parent only needs the read end.
    writeEnd.reset();

    int execError = 0;
    std::size_t bytesRead = 0;

    // Read the exec failure information, if any.
    while (bytesRead < sizeof(execError))
    {
        const ssize_t count = read(
            readEnd.get(),
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

            // Reap the child before reporting the read failure.
            int ignoredStatus = 0;
            struct rusage ignoredUsage {};

            pid_t waitResult;

            do
            {
                waitResult = wait4(
                    childPid,
                    &ignoredStatus,
                    0,
                    &ignoredUsage
                );
            }
            while (waitResult == -1 && errno == EINTR);

            throw std::system_error(
                error,
                std::generic_category(),
                "Reading exec error pipe failed"
            );
        }
    }

    // Close the parent's read descriptor.
    // If an exception occurs before this point, RAII handles cleanup.
    readEnd.reset();

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
        const int error = errno;

        throw std::system_error(
            error,
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