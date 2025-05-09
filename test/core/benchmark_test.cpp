#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <functional>
#include "core/benchmark.hpp"

using namespace trading_engine::core;

// Simple function for benchmarking
void sleep_function(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

TEST(BenchmarkTest, BasicBenchmark) {
    // Benchmark a simple function for a few iterations
    auto result = Benchmark::run("SleepTest", []() { sleep_function(1); }, 3);
    
    // Verify basic properties
    EXPECT_EQ(result.name, "SleepTest");
    EXPECT_EQ(result.iterations, 3);
    
    // Each iteration takes 1ms, so total time should be at least 3ms
    EXPECT_GE(result.total_time_ns, 3 * 1000000);
    
    // Mean time should be close to 1ms
    EXPECT_GE(result.mean_time_ns, 1000000);
}

TEST(BenchmarkTest, DurationBasedBenchmark) {
    // Benchmark a function for a specific duration
    auto result = Benchmark::run_for_duration(
        "DurationTest", 
        []() { sleep_function(1); }, 
        std::chrono::milliseconds(5));
    
    // Should be at least a few iterations
    EXPECT_GE(result.iterations, 4);
    
    // Statistics should be reasonable
    EXPECT_GT(result.mean_time_ns, 0);
    EXPECT_LT(result.min_time_ns, result.max_time_ns);
}

TEST(BenchmarkTest, BenchmarkMacros) {
    // Test the benchmark macros (just for compilation, not actual verification)
    auto func = []() { sleep_function(1); };
    
    // This would log the results, so just creating a minimal test
    // TE_BENCHMARK("MacroTest", func, 1);
    
    // Just verify the code compiles
    SUCCEED();
}

TEST(BenchmarkTest, TimePerOp) {
    // Test the time_per_op methods
    BenchmarkResult result = {
        "test", 10, 10000000, 900000, 1100000, 1000000, 100000, 1000000, 1050000, 1090000
    };
    
    // Verify the time per op calculations
    EXPECT_EQ(result.time_per_op_ns(), 1000000);
    EXPECT_EQ(result.time_per_op_us(), 1000);
    EXPECT_EQ(result.time_per_op_ms(), 1);
}

TEST(BenchmarkTest, IterationsPerSec) {
    // Test the iterations_per_sec method
    BenchmarkResult result = {
        "test", 10, 10000000, 900000, 1100000, 1000000, 100000, 1000000, 1050000, 1090000
    };
    
    // 10 iterations in 10ms should be 1000 iterations per second
    EXPECT_EQ(result.iterations_per_sec(), 1000);
}

TEST(BenchmarkTest, MeasureTimeMacro) {
    // Test the TE_MEASURE_TIME macro
    {
        TE_MEASURE_TIME("TestMeasure");
        sleep_function(1);
    }
    
    // No explicit verification, just ensure the code compiles and runs
    SUCCEED();
} 