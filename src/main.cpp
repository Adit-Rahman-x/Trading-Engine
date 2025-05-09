#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include "core/timer.hpp"
#include "core/logger.hpp"
#include "core/benchmark.hpp"

using namespace trading_engine::core;

// Simple function to benchmark
void test_function() {
    // Simulate some work
    volatile int sum = 0;
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
}

int main() {
    // Set up logging
    global_logger().set_min_level(LogLevel::INFO);
    global_logger().set_file_output("trading_engine.log");
    
    TE_LOG_INFO("Trading Engine starting up...");
    
    // Test timer
    {
        TE_LOG_INFO("Testing Timer...");
        
        Timer timer;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        TE_LOG_INFO("Elapsed time: %ld ms", timer.elapsed_ms());
        TE_LOG_INFO("Current timestamp: %s", Timer::timestamp().c_str());
    }
    
    // Test benchmarking
    {
        TE_LOG_INFO("Testing Benchmark...");
        
        // Run a benchmark for 1000 iterations
        auto result = Benchmark::run("TestFunction", test_function, 10000);
        Benchmark::log_result(result);
        
        // Measure a block of code
        {
            TE_MEASURE_TIME("MeasuredBlock");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            test_function();
        }
    }
    
    TE_LOG_INFO("All tests completed successfully");
    return 0;
} 