#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <utility>
#include "core/timer.hpp"
#include "core/logger.hpp"

namespace trading_engine {
namespace core {

/**
 * Benchmark result containing statistics about a benchmark run
 */
struct BenchmarkResult {
    std::string name;
    int64_t iterations;
    int64_t total_time_ns;
    int64_t min_time_ns;
    int64_t max_time_ns;
    double mean_time_ns;
    double stddev_time_ns;
    double median_time_ns;
    double p90_time_ns;
    double p99_time_ns;
    
    double iterations_per_sec() const {
        return (static_cast<double>(iterations) * 1e9) / static_cast<double>(total_time_ns);
    }
    
    double time_per_op_ns() const {
        return mean_time_ns;
    }
    
    double time_per_op_us() const {
        return mean_time_ns / 1000.0;
    }
    
    double time_per_op_ms() const {
        return mean_time_ns / 1000000.0;
    }
};

/**
 * Benchmarking utility for measuring performance of functions
 */
class Benchmark {
public:
    // Run a benchmark for a given number of iterations
    template <typename Func>
    static BenchmarkResult run(const std::string& name, Func&& func, int64_t iterations) {
        std::vector<int64_t> times;
        times.reserve(iterations);
        
        int64_t total_time = 0;
        
        // Warm-up run
        func();
        
        // Actual benchmark runs
        for (int64_t i = 0; i < iterations; ++i) {
            Timer timer;
            func();
            int64_t elapsed = timer.elapsed_ns();
            times.push_back(elapsed);
            total_time += elapsed;
        }
        
        // Calculate statistics
        std::sort(times.begin(), times.end());
        
        int64_t min_time = times.front();
        int64_t max_time = times.back();
        double mean_time = static_cast<double>(total_time) / iterations;
        
        // Calculate standard deviation
        double variance = 0.0;
        for (int64_t time : times) {
            double diff = static_cast<double>(time) - mean_time;
            variance += diff * diff;
        }
        variance /= iterations;
        double stddev = std::sqrt(variance);
        
        // Calculate percentiles
        double median = (iterations % 2 == 0) 
            ? (times[iterations/2 - 1] + times[iterations/2]) / 2.0 
            : times[iterations/2];
        
        size_t p90_idx = static_cast<size_t>(iterations * 0.9);
        size_t p99_idx = static_cast<size_t>(iterations * 0.99);
        
        double p90 = times[p90_idx];
        double p99 = times[p99_idx];
        
        return {
            name, iterations, total_time, min_time, max_time,
            mean_time, stddev, median, p90, p99
        };
    }
    
    // Run a timed benchmark for a given duration
    template <typename Func>
    static BenchmarkResult run_for_duration(const std::string& name, Func&& func, 
                                           std::chrono::milliseconds duration) {
        // Estimate how many iterations we need to run
        constexpr int CALIBRATION_ITERATIONS = 10;
        int64_t calibration_time = 0;
        
        // Run a few iterations to calibrate
        for (int i = 0; i < CALIBRATION_ITERATIONS; ++i) {
            Timer timer;
            func();
            calibration_time += timer.elapsed_ns();
        }
        
        // Calculate iterations based on target duration
        int64_t avg_time = calibration_time / CALIBRATION_ITERATIONS;
        int64_t target_ns = duration.count() * 1000000;
        int64_t estimated_iterations = target_ns / avg_time;
        
        // Run at least 10 iterations
        int64_t iterations = std::max<int64_t>(10, estimated_iterations);
        
        return run(name, std::forward<Func>(func), iterations);
    }
    
    // Log benchmark results
    static void log_result(const BenchmarkResult& result) {
        TE_LOG_INFO("Benchmark: %s", result.name.c_str());
        TE_LOG_INFO("  Iterations: %ld", result.iterations);
        TE_LOG_INFO("  Total time: %.3f ms", result.total_time_ns / 1e6);
        TE_LOG_INFO("  Throughput: %.2f ops/sec", result.iterations_per_sec());
        TE_LOG_INFO("  Time per op: %.3f us (mean)", result.time_per_op_us());
        TE_LOG_INFO("  Min: %.3f us", result.min_time_ns / 1e3);
        TE_LOG_INFO("  Max: %.3f us", result.max_time_ns / 1e3);
        TE_LOG_INFO("  Stddev: %.3f us", result.stddev_time_ns / 1e3);
        TE_LOG_INFO("  Median: %.3f us", result.median_time_ns / 1e3);
        TE_LOG_INFO("  p90: %.3f us", result.p90_time_ns / 1e3);
        TE_LOG_INFO("  p99: %.3f us", result.p99_time_ns / 1e3);
    }
};

} // namespace core
} // namespace trading_engine

// Convenience macros for benchmarking
#define TE_BENCHMARK(name, func, iterations) \
    ::trading_engine::core::Benchmark::log_result( \
        ::trading_engine::core::Benchmark::run(name, func, iterations))

#define TE_BENCHMARK_DURATION(name, func, ms) \
    ::trading_engine::core::Benchmark::log_result( \
        ::trading_engine::core::Benchmark::run_for_duration( \
            name, func, std::chrono::milliseconds(ms)))

// Measure execution time of a block of code
#define TE_MEASURE_TIME(name) \
    auto TE_MEASURE_TIME_start = ::trading_engine::core::Timer::now_ns(); \
    auto TE_MEASURE_TIME_name = name; \
    auto TE_MEASURE_TIME_guard = ::std::unique_ptr<void, std::function<void(void*)>>( \
        (void*)1, [&](void*) { \
            auto end = ::trading_engine::core::Timer::now_ns(); \
            auto duration = end - TE_MEASURE_TIME_start; \
            TE_LOG_INFO("%s took %ld ns (%.3f µs)", \
                TE_MEASURE_TIME_name, duration, duration / 1000.0); \
        }); 