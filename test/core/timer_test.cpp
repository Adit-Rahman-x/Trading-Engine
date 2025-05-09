#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "core/timer.hpp"

using namespace trading_engine::core;

TEST(TimerTest, ElapsedTimeIsAccurate) {
    Timer timer;
    
    // Sleep for a known duration
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Check that elapsed time is at least the sleep duration
    EXPECT_GE(timer.elapsed_ms(), 10);
    
    // Typically it should be very close to the sleep time, but allow a small margin
    EXPECT_LE(timer.elapsed_ms(), 15);
}

TEST(TimerTest, Reset) {
    Timer timer;
    
    // Sleep for a short period
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    
    // Record the elapsed time
    auto first_elapsed = timer.elapsed_ms();
    EXPECT_GE(first_elapsed, 5);
    
    // Reset the timer
    timer.reset();
    
    // Check that elapsed time is reset and close to zero
    EXPECT_LT(timer.elapsed_ms(), 5);
    
    // Sleep again
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    
    // Check that elapsed time is from reset
    EXPECT_GE(timer.elapsed_ms(), 5);
    EXPECT_LT(timer.elapsed_ms(), 10);
}

TEST(TimerTest, StaticNowMethods) {
    // Test that static now methods return increasing values
    auto now1_ns = Timer::now_ns();
    auto now1_us = Timer::now_us();
    auto now1_ms = Timer::now_ms();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    
    auto now2_ns = Timer::now_ns();
    auto now2_us = Timer::now_us();
    auto now2_ms = Timer::now_ms();
    
    EXPECT_GT(now2_ns, now1_ns);
    EXPECT_GT(now2_us, now1_us);
    EXPECT_GT(now2_ms, now1_ms);
}

TEST(TimerTest, Timestamp) {
    // Check that timestamp format matches expectations
    std::string timestamp = Timer::timestamp();
    
    // Basic format check: YYYY-MM-DD HH:MM:SS.nnnnnnnnn
    EXPECT_EQ(timestamp.size(), 29);
    EXPECT_EQ(timestamp[4], '-');
    EXPECT_EQ(timestamp[7], '-');
    EXPECT_EQ(timestamp[10], ' ');
    EXPECT_EQ(timestamp[13], ':');
    EXPECT_EQ(timestamp[16], ':');
    EXPECT_EQ(timestamp[19], '.');
} 