#include <iostream>
#include <stdexcept>
#include <vector>
#include <chrono>

#include <unistd.h>

#include "profiler/ProcessMonitor.h"

namespace{
    void check(bool condition, const char* message){
        if(!condition){
            throw std::runtime_error(message);
        }
    }

    void testCollectMultipleSamples(){
        std::cout << "Running : testCollectMultipleSamples" << std::endl;

        profiler::ProcfsReader reader;

        profiler::ProcessMonitor monitor(reader);

        const pid_t currentPid = ::getpid();

        std::vector<profiler::ProcessMetrics> samples;

        monitor.monitor(
            currentPid,
            std::chrono::milliseconds(100),
            3,
            [&samples](const profiler::ProcessMetrics& metrics){
                samples.push_back(metrics);
            }
        );

        check(
            samples.size() == 3, 
            "Expected exactly three samples"
        );

        for(const auto& sample : samples){
            check(
                sample.pid == currentPid,
                "Incorrect PID in sample"
            );

            check(
                sample.state != '\0',
                "Invalid process state"
            );
        }

        check(
            samples[0].timestamp < samples[1].timestamp,
            "Second sample timestamp must be later"
        );

        check(
            samples[1].timestamp < samples[2].timestamp,
            "Third sample timestamp must be later"
        );

        std::cout << "Collected samples: "
            << samples.size() << '\n';

        std::cout << "PASSED: testCollectMultipleSamples\n\n";
        
    }
        
    void testInvalidSamplingInterval()
    {
        std::cout << "Running: testInvalidSamplingInterval\n";

        profiler::ProcfsReader reader;
        profiler::ProcessMonitor monitor(reader);

        bool exceptionThrown = false;

        try
        {
            monitor.monitor(
                ::getpid(),
                std::chrono::milliseconds(0),
                3,
                [](const profiler::ProcessMetrics&) {}
            );
        }
        catch (const std::invalid_argument&)
        {
            exceptionThrown = true;
        }

        check(
            exceptionThrown,
            "Expected exception for invalid sampling interval"
        );

        std::cout << "PASSED: testInvalidSamplingInterval\n\n";
    }

    void testZeroSamples()
    {
        std::cout << "Running: testZeroSamples\n";

        profiler::ProcfsReader reader;
        profiler::ProcessMonitor monitor(reader);

        bool exceptionThrown = false;

        try
        {
            monitor.monitor(
                ::getpid(),
                std::chrono::milliseconds(100),
                0,
                [](const profiler::ProcessMetrics&) {}
            );
        }
        catch (const std::invalid_argument&)
        {
            exceptionThrown = true;
        }

        check(
            exceptionThrown,
            "Expected exception for zero sample count"
        );

        std::cout << "PASSED: testZeroSamples\n\n";
    }
}
    int main(){
        try{
            testCollectMultipleSamples();
            testInvalidSamplingInterval();
            //testZeroSamples();

            std::cout << "All ProcessMonitor tests passed!\n";
            return 0;
        } catch(const std::exception& exception){
            std::cerr << "TEST FAILURE : " << exception.what() << std::endl;
            return 1;
        }
        return 0;
    }