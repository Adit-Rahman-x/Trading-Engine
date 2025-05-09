#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <utility>

namespace trading_engine {
namespace core {

/**
 * High-resolution timer utility for performance-critical applications.
 * Provides nanosecond precision timing capabilities.
 */
class Timer {
public:
    // Clock types
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;

    // Constructor starts the timer
    Timer() : start_time_(Clock::now()) {}

    // Reset the timer
    void reset() {
        start_time_ = Clock::now();
    }

    // Get elapsed time in various units
    int64_t elapsed_ns() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            Clock::now() - start_time_).count();
    }

    int64_t elapsed_us() const {
        return std::chrono::duration_cast<std::chrono::microseconds>(
            Clock::now() - start_time_).count();
    }

    int64_t elapsed_ms() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - start_time_).count();
    }

    double elapsed_seconds() const {
        return std::chrono::duration_cast<std::chrono::duration<double>>(
            Clock::now() - start_time_).count();
    }

    // Get current time since epoch in nanoseconds
    static int64_t now_ns() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            Clock::now().time_since_epoch()).count();
    }

    // Get current time since epoch in microseconds
    static int64_t now_us() {
        return std::chrono::duration_cast<std::chrono::microseconds>(
            Clock::now().time_since_epoch()).count();
    }

    // Get current time since epoch in milliseconds
    static int64_t now_ms() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now().time_since_epoch()).count();
    }

    // Get current timestamp for logging (formatted as needed)
    static std::string timestamp() {
        auto now = std::chrono::system_clock::now();
        auto now_time = std::chrono::system_clock::to_time_t(now);
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()).count() % 1000000000;

        char time_str[32];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));
        
        char buffer[48];
        std::snprintf(buffer, sizeof(buffer), "%s.%09ld", time_str, ns);
        return std::string(buffer);
    }

private:
    TimePoint start_time_;
};

} // namespace core
} // namespace trading_engine 