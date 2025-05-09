#pragma once

#include <atomic>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <array>
#include <thread>
#include <vector>
#include <optional>
#include <cstdarg>
#include <utility>
#include <memory>
#include <fstream>
#include "core/timer.hpp"

namespace trading_engine {
namespace core {

/**
 * LogLevel enum for controlling logging verbosity
 */
enum class LogLevel : uint8_t {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5,
    OFF = 6
};

/**
 * Simple string representation of LogLevel
 */
constexpr std::string_view to_string(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        case LogLevel::OFF:   return "OFF  ";
        default:              return "UNKWN";
    }
}

/**
 * Lock-free ring buffer for log messages
 */
class LogRingBuffer {
public:
    static constexpr size_t LOG_ENTRY_SIZE = 1024;
    using LogEntry = std::array<char, LOG_ENTRY_SIZE>;

    explicit LogRingBuffer(size_t capacity) 
        : buffer_(capacity), 
          write_index_(0), 
          read_index_(0) {}

    bool try_write(std::string_view log_message) {
        const size_t current_write = write_index_.load(std::memory_order_relaxed);
        const size_t next_write = (current_write + 1) % buffer_.size();
        
        if (next_write == read_index_.load(std::memory_order_acquire)) {
            return false; // Buffer is full
        }

        auto& entry = buffer_[current_write];
        const size_t copy_size = std::min(log_message.size(), LOG_ENTRY_SIZE - 1);
        std::memcpy(entry.data(), log_message.data(), copy_size);
        entry[copy_size] = '\0';
        
        write_index_.store(next_write, std::memory_order_release);
        return true;
    }

    std::optional<std::string_view> try_read() {
        const size_t current_read = read_index_.load(std::memory_order_relaxed);
        
        if (current_read == write_index_.load(std::memory_order_acquire)) {
            return std::nullopt; // Buffer is empty
        }
        
        std::string_view result(buffer_[current_read].data());
        
        const size_t next_read = (current_read + 1) % buffer_.size();
        read_index_.store(next_read, std::memory_order_release);
        
        return result;
    }

private:
    std::vector<LogEntry> buffer_;
    std::atomic<size_t> write_index_;
    std::atomic<size_t> read_index_;
};

/**
 * High-performance lock-free logger implementation
 */
class Logger {
public:
    static constexpr size_t DEFAULT_BUFFER_SIZE = 8192;
    
    explicit Logger(LogLevel min_level = LogLevel::INFO, 
                   size_t buffer_size = DEFAULT_BUFFER_SIZE)
        : min_level_(min_level), 
          buffer_(buffer_size), 
          is_running_(true) {
        flush_thread_ = std::thread(&Logger::flush_worker, this);
    }

    ~Logger() {
        is_running_.store(false, std::memory_order_relaxed);
        if (flush_thread_.joinable()) {
            flush_thread_.join();
        }
    }

    void set_file_output(const std::string& filename) {
        file_output_ = std::make_unique<std::ofstream>(filename, std::ios::out | std::ios::app);
    }

    void set_min_level(LogLevel level) {
        min_level_.store(level, std::memory_order_relaxed);
    }

    // Log with format string
    void log(LogLevel level, const char* format, ...) {
        if (level < min_level_.load(std::memory_order_relaxed)) {
            return;
        }

        // Format timestamp and log level
        char buffer[LogRingBuffer::LOG_ENTRY_SIZE];
        int prefix_len = std::snprintf(buffer, sizeof(buffer), 
                                      "[%s] [%s] [%zu] ", 
                                      Timer::timestamp().c_str(), 
                                      to_string(level).data(),
                                      std::hash<std::thread::id>{}(std::this_thread::get_id()));
        
        // Format the user message
        va_list args;
        va_start(args, format);
        int msg_len = std::vsnprintf(buffer + prefix_len, 
                                     sizeof(buffer) - prefix_len, 
                                     format, args);
        va_end(args);
        
        if (msg_len < 0) {
            return; // Formatting error
        }

        // Try to write to buffer
        if (!buffer_.try_write(std::string_view(buffer, prefix_len + msg_len))) {
            // Buffer full - could implement fallback strategy here
        }
    }

    // Convenience methods for different log levels
    void trace(const char* format, ...) {
        if (LogLevel::TRACE < min_level_.load(std::memory_order_relaxed)) {
            return;
        }
        
        va_list args;
        va_start(args, format);
        char buffer[LogRingBuffer::LOG_ENTRY_SIZE];
        int prefix_len = std::snprintf(buffer, sizeof(buffer), 
                                      "[%s] [%s] [%zu] ", 
                                      Timer::timestamp().c_str(), 
                                      to_string(LogLevel::TRACE).data(),
                                      std::hash<std::thread::id>{}(std::this_thread::get_id()));
        
        int msg_len = std::vsnprintf(buffer + prefix_len, 
                                     sizeof(buffer) - prefix_len, 
                                     format, args);
        va_end(args);
        
        if (msg_len >= 0) {
            buffer_.try_write(std::string_view(buffer, prefix_len + msg_len));
        }
    }

    void debug(const char* format, ...) {
        if (LogLevel::DEBUG < min_level_.load(std::memory_order_relaxed)) {
            return;
        }
        
        va_list args;
        va_start(args, format);
        char buffer[LogRingBuffer::LOG_ENTRY_SIZE];
        int prefix_len = std::snprintf(buffer, sizeof(buffer), 
                                      "[%s] [%s] [%zu] ", 
                                      Timer::timestamp().c_str(), 
                                      to_string(LogLevel::DEBUG).data(),
                                      std::hash<std::thread::id>{}(std::this_thread::get_id()));
        
        int msg_len = std::vsnprintf(buffer + prefix_len, 
                                     sizeof(buffer) - prefix_len, 
                                     format, args);
        va_end(args);
        
        if (msg_len >= 0) {
            buffer_.try_write(std::string_view(buffer, prefix_len + msg_len));
        }
    }

    void info(const char* format, ...) {
        if (LogLevel::INFO < min_level_.load(std::memory_order_relaxed)) {
            return;
        }
        
        va_list args;
        va_start(args, format);
        char buffer[LogRingBuffer::LOG_ENTRY_SIZE];
        int prefix_len = std::snprintf(buffer, sizeof(buffer), 
                                      "[%s] [%s] [%zu] ", 
                                      Timer::timestamp().c_str(), 
                                      to_string(LogLevel::INFO).data(),
                                      std::hash<std::thread::id>{}(std::this_thread::get_id()));
        
        int msg_len = std::vsnprintf(buffer + prefix_len, 
                                     sizeof(buffer) - prefix_len, 
                                     format, args);
        va_end(args);
        
        if (msg_len >= 0) {
            buffer_.try_write(std::string_view(buffer, prefix_len + msg_len));
        }
    }

    void warn(const char* format, ...) {
        if (LogLevel::WARN < min_level_.load(std::memory_order_relaxed)) {
            return;
        }
        
        va_list args;
        va_start(args, format);
        char buffer[LogRingBuffer::LOG_ENTRY_SIZE];
        int prefix_len = std::snprintf(buffer, sizeof(buffer), 
                                      "[%s] [%s] [%zu] ", 
                                      Timer::timestamp().c_str(), 
                                      to_string(LogLevel::WARN).data(),
                                      std::hash<std::thread::id>{}(std::this_thread::get_id()));
        
        int msg_len = std::vsnprintf(buffer + prefix_len, 
                                     sizeof(buffer) - prefix_len, 
                                     format, args);
        va_end(args);
        
        if (msg_len >= 0) {
            buffer_.try_write(std::string_view(buffer, prefix_len + msg_len));
        }
    }

    void error(const char* format, ...) {
        if (LogLevel::ERROR < min_level_.load(std::memory_order_relaxed)) {
            return;
        }
        
        va_list args;
        va_start(args, format);
        char buffer[LogRingBuffer::LOG_ENTRY_SIZE];
        int prefix_len = std::snprintf(buffer, sizeof(buffer), 
                                      "[%s] [%s] [%zu] ", 
                                      Timer::timestamp().c_str(), 
                                      to_string(LogLevel::ERROR).data(),
                                      std::hash<std::thread::id>{}(std::this_thread::get_id()));
        
        int msg_len = std::vsnprintf(buffer + prefix_len, 
                                     sizeof(buffer) - prefix_len, 
                                     format, args);
        va_end(args);
        
        if (msg_len >= 0) {
            buffer_.try_write(std::string_view(buffer, prefix_len + msg_len));
        }
    }

    void fatal(const char* format, ...) {
        if (LogLevel::FATAL < min_level_.load(std::memory_order_relaxed)) {
            return;
        }
        
        va_list args;
        va_start(args, format);
        char buffer[LogRingBuffer::LOG_ENTRY_SIZE];
        int prefix_len = std::snprintf(buffer, sizeof(buffer), 
                                      "[%s] [%s] [%zu] ", 
                                      Timer::timestamp().c_str(), 
                                      to_string(LogLevel::FATAL).data(),
                                      std::hash<std::thread::id>{}(std::this_thread::get_id()));
        
        int msg_len = std::vsnprintf(buffer + prefix_len, 
                                     sizeof(buffer) - prefix_len, 
                                     format, args);
        va_end(args);
        
        if (msg_len >= 0) {
            buffer_.try_write(std::string_view(buffer, prefix_len + msg_len));
        }
    }

private:
    // Worker thread that flushes log entries
    void flush_worker() {
        while (is_running_.load(std::memory_order_relaxed)) {
            // Process any available log entries
            while (auto entry = buffer_.try_read()) {
                // Write to stdout
                std::fprintf(stdout, "%s\n", entry->data());
                
                // Write to file if configured
                if (file_output_ && file_output_->is_open()) {
                    (*file_output_) << entry->data() << std::endl;
                }
            }
            
            // Sleep briefly to avoid burning CPU
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        // Final flush on shutdown
        while (auto entry = buffer_.try_read()) {
            std::fprintf(stdout, "%s\n", entry->data());
            if (file_output_ && file_output_->is_open()) {
                (*file_output_) << entry->data() << std::endl;
            }
        }
    }

    std::atomic<LogLevel> min_level_;
    LogRingBuffer buffer_;
    std::atomic<bool> is_running_;
    std::thread flush_thread_;
    std::unique_ptr<std::ofstream> file_output_;
};

// Global logger instance
inline Logger& global_logger() {
    static Logger instance;
    return instance;
}

} // namespace core
} // namespace trading_engine

// Convenience macros
#define TE_LOG_TRACE(...) ::trading_engine::core::global_logger().trace(__VA_ARGS__)
#define TE_LOG_DEBUG(...) ::trading_engine::core::global_logger().debug(__VA_ARGS__)
#define TE_LOG_INFO(...)  ::trading_engine::core::global_logger().info(__VA_ARGS__)
#define TE_LOG_WARN(...)  ::trading_engine::core::global_logger().warn(__VA_ARGS__)
#define TE_LOG_ERROR(...) ::trading_engine::core::global_logger().error(__VA_ARGS__)
#define TE_LOG_FATAL(...) ::trading_engine::core::global_logger().fatal(__VA_ARGS__) 