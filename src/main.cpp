
#include <iostream>
#include <chrono>
#include <cerrno>
#include <cstdio>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>

int main()
{
    // Arguments for the external application.
    char arg0[] = "sleep";
    char arg1[] = "3";

    char* args[] = {
        arg0,
        arg1,
        nullptr
    };

    std::cout << "Profiler started\n";

    // Start measuring wall-clock time.
    const auto startTime = std::chrono::steady_clock::now();

    // Create a child process.
    pid_t childPid = fork();

    if (childPid < 0)
    {
        perror("fork failed");
        return 1;
    }

    if (childPid == 0)
    {
        // Child process.

        std::cout << "Child process started. PID: "
                  << getpid() << '\n';

        // Replace child process with the sleep application.
        execvp("/usr/bin/sleep", args);

        // Reaching here means execvp failed.
        perror("execvp failed");

        // Exit without running normal C++ cleanup handlers.
        _exit(127);
    }

    // Parent process.

    std::cout << "Parent created child. PID: "
              << childPid << '\n';

    int status = 0;
    struct rusage usage {};

    // Wait for the child and collect resource usage.
    pid_t result;

    do
    {
        result = wait4(childPid, &status, 0, &usage);
    }
    while (result == -1 && errno == EINTR);

    if (result == -1)
    {
        perror("wait4 failed");
        return 1;
    }

    const auto endTime = std::chrono::steady_clock::now();

    const std::chrono::duration<double> elapsedTime =
        endTime - startTime;

    std::cout << "\n========== Profiling Result ==========\n";

    std::cout << "Child PID: " << childPid << '\n';

    std::cout << "Wall-clock time: "
              << elapsedTime.count() << " seconds\n";

    const double userCpuTime =
        usage.ru_utime.tv_sec +
        usage.ru_utime.tv_usec / 1000000.0;

    const double systemCpuTime =
        usage.ru_stime.tv_sec +
        usage.ru_stime.tv_usec / 1000000.0;

    std::cout << "User CPU time: "
              << userCpuTime << " seconds\n";

    std::cout << "System CPU time: "
              << systemCpuTime << " seconds\n";

    std::cout << "Maximum RSS: "
              << usage.ru_maxrss / 1024.0 << " MB\n";

    if (WIFEXITED(status))
    {
        std::cout << "Exit code: "
                  << WEXITSTATUS(status) << '\n';
    }
    else if (WIFSIGNALED(status))
    {
        std::cout << "Terminated by signal: "
                  << WTERMSIG(status) << '\n';
    }

    std::cout << "======================================\n";

    return 0;
}
