#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <fstream>
#include <filesystem>
#include <chrono>
#include "core/logger.hpp"

using namespace trading_engine::core;

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset test log file
        if (std::filesystem::exists(test_log_file)) {
            std::filesystem::remove(test_log_file);
        }
    }

    void TearDown() override {
        // Clean up test file
        if (std::filesystem::exists(test_log_file)) {
            std::filesystem::remove(test_log_file);
        }
    }

    const std::string test_log_file = "logger_test.log";
};

TEST_F(LoggerTest, LoggingToFile) {
    // Create a logger with file output
    Logger logger(LogLevel::INFO);
    logger.set_file_output(test_log_file);
    
    // Log a test message
    const char* test_message = "Test log message";
    logger.info("%s", test_message);
    
    // Give some time for the async logger to flush
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Verify that the file contains the message
    std::ifstream log_file(test_log_file);
    ASSERT_TRUE(log_file.is_open());
    
    std::string line;
    bool found = false;
    while (std::getline(log_file, line)) {
        if (line.find(test_message) != std::string::npos) {
            found = true;
            break;
        }
    }
    
    EXPECT_TRUE(found) << "Test message not found in log file";
}

TEST_F(LoggerTest, LogLevelFiltering) {
    // Create a logger with minimum level WARNING
    Logger logger(LogLevel::WARN);
    logger.set_file_output(test_log_file);
    
    // Log messages of different levels
    logger.info("This should not be logged");
    logger.warn("This warning should be logged");
    logger.error("This error should be logged");
    
    // Give some time for the async logger to flush
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Verify that only warning and error messages are in the file
    std::ifstream log_file(test_log_file);
    ASSERT_TRUE(log_file.is_open());
    
    std::string line;
    int count = 0;
    bool found_warning = false;
    bool found_error = false;
    bool found_info = false;
    
    while (std::getline(log_file, line)) {
        count++;
        if (line.find("warning should be logged") != std::string::npos) {
            found_warning = true;
        }
        if (line.find("error should be logged") != std::string::npos) {
            found_error = true;
        }
        if (line.find("should not be logged") != std::string::npos) {
            found_info = true;
        }
    }
    
    EXPECT_TRUE(found_warning) << "Warning message not found";
    EXPECT_TRUE(found_error) << "Error message not found";
    EXPECT_FALSE(found_info) << "Info message should not have been logged";
    EXPECT_EQ(count, 2) << "Expected exactly 2 log entries";
}

TEST_F(LoggerTest, ChangingLogLevel) {
    // Create a logger with minimum level WARNING
    Logger logger(LogLevel::WARN);
    logger.set_file_output(test_log_file);
    
    // Log a message that should be filtered out
    logger.info("This info should not be logged");
    
    // Change log level to INFO
    logger.set_min_level(LogLevel::INFO);
    
    // Log another INFO message that should now appear
    logger.info("This info should be logged");
    
    // Give some time for the async logger to flush
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Verify that only the second message is in the file
    std::ifstream log_file(test_log_file);
    ASSERT_TRUE(log_file.is_open());
    
    std::string line;
    bool found_first_message = false;
    bool found_second_message = false;
    
    while (std::getline(log_file, line)) {
        if (line.find("should not be logged") != std::string::npos) {
            found_first_message = true;
        }
        if (line.find("should be logged") != std::string::npos) {
            found_second_message = true;
        }
    }
    
    EXPECT_FALSE(found_first_message) << "First message should not have been logged";
    EXPECT_TRUE(found_second_message) << "Second message not found";
}

TEST(LoggerGlobalTest, GlobalLoggerInstance) {
    // Test that the global logger instance works
    Logger& global = global_logger();
    
    // Test that we can access and set properties
    global.set_min_level(LogLevel::DEBUG);
    
    // Just verify it doesn't crash
    TE_LOG_INFO("Test global logger macro");
    
    // No explicit verification, just ensure the code compiles and runs
    SUCCEED();
} 