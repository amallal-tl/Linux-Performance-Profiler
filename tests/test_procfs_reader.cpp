#include <iostream>
#include <stdexcept>

#include <unistd.h>

#include "profiler/ProcfsReader.h"

namespace {
    void check(bool condition, const char* message){
        if(!condition){
            throw std::runtime_error(message);
        }
    }

    void testReadCurrentProcess(){
        std::cout << "Running : testReadCurrentProcess\n";

        profiler::ProcfsReader reader;

        const pid_t current_pid = ::getpid();
        const pid_t expectedParentPid = ::getppid();

        const profiler::ProcessMetrics metrics = reader.readProcessMetrics(current_pid);
        
        check(
            metrics.pid == current_pid,
            "Incorrect process PID"
        );

        check(
            metrics.parentPid == expectedParentPid,
            "Invalid Parent PID"
        );

         check(
            metrics.state != '\0',
            "Invalid process state"
        );

        check(
            metrics.threadCount >= 1,
            "Invalid thread count"
        );

        check(
            metrics.virtualMemoryBytes > 0,
            "Invalid virtual memory value"
        );

        check(
            metrics.residentPages >= 0,
            "Invalid resident page count"
        );

        std::cout << "PID: " << metrics.pid << '\n';
        std::cout << "Parent PID: " << metrics.parentPid << '\n';
        std::cout << "State: " << metrics.state << '\n';
        std::cout << "Threads: " << metrics.threadCount << '\n';

        std::cout << "User CPU ticks: "
                << metrics.userCpuTicks << '\n';

        std::cout << "System CPU ticks: "
                << metrics.systemCpuTicks << '\n';

        std::cout << "Virtual memory: "
                << metrics.virtualMemoryBytes << " bytes\n";

        std::cout << "Resident pages: "
                << metrics.residentPages << '\n';

        std::cout << "PASSED: testReadCurrentProcess\n\n";
    }

    void testInvalidProcess(){
        std::cout << "Running : testInvalidProcess\n";

        profiler::ProcfsReader reader;

        bool exceptionThrown = false;

        try{
            reader.readProcessMetrics(-1);
        } catch (const std::runtime_error&) {
            exceptionThrown = true;
        }

        check(
            exceptionThrown,
            "Expected an exception for invalid PID"
        );

        std::cout << "PASSED : TestInvalidProcess\n\n";
    }

    int main(){
        try{
            testReadCurrentProcess();

            testInvalidProcess();

            std::cout << "All tests Passed\n\n";
        } catch (const std::runtime_error& exception){
            std::cerr << "TEST Failure " << exception.what() << std::endl;
        }
    
        return 1;
    }
}