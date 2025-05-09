#pragma once

#include <cstdint>
#include <string>
#include <chrono>
#include <limits>
#include <array>
#include <functional>

namespace trading_engine {
namespace orderbook {

// Forward declarations
class Order;
class PriceLevel;
class OrderBook;

/**
 * Order ID type - 64-bit unsigned integer
 * Uniquely identifies an order in the system
 */
using OrderId = uint64_t;
constexpr OrderId INVALID_ORDER_ID = 0;

/**
 * Symbol - identifier for a security/instrument
 */
using Symbol = std::string;

/**
 * Price - fixed point decimal for deterministic arithmetic
 * Represents price in the smallest currency unit (e.g. cents)
 * Scale factor of 10000 means a value of 12345 represents $1.2345
 */
class Price {
public:
    static constexpr int64_t SCALE_FACTOR = 10000;
    static constexpr Price INVALID = Price(std::numeric_limits<int64_t>::lowest());
    static constexpr Price MAX_VALUE = Price(std::numeric_limits<int64_t>::max());
    static constexpr Price MIN_VALUE = Price(std::numeric_limits<int64_t>::min());
    static constexpr Price ZERO = Price(0);

    // Default constructor
    constexpr Price() : value_(0) {}
    
    // Integer constructor
    constexpr explicit Price(int64_t value) : value_(value) {}
    
    // Double constructor - converts to fixed point
    explicit Price(double value) : value_(static_cast<int64_t>(value * SCALE_FACTOR)) {}
    
    // Convert to double
    double to_double() const { return static_cast<double>(value_) / SCALE_FACTOR; }
    
    // Get raw fixed point value
    constexpr int64_t raw_value() const { return value_; }
    
    // Arithmetic operators
    Price operator+(const Price& other) const { return Price(value_ + other.value_); }
    Price operator-(const Price& other) const { return Price(value_ - other.value_); }
    Price operator*(int64_t multiplier) const { return Price(value_ * multiplier); }
    Price operator/(int64_t divisor) const { return Price(value_ / divisor); }
    
    // Comparison operators
    bool operator==(const Price& other) const { return value_ == other.value_; }
    bool operator!=(const Price& other) const { return value_ != other.value_; }
    bool operator<(const Price& other) const { return value_ < other.value_; }
    bool operator<=(const Price& other) const { return value_ <= other.value_; }
    bool operator>(const Price& other) const { return value_ > other.value_; }
    bool operator>=(const Price& other) const { return value_ >= other.value_; }
    
    // Output as a string with proper decimal places
    std::string to_string() const;

private:
    int64_t value_;  // Raw fixed point value
};

/**
 * Quantity - fixed point decimal for deterministic arithmetic
 * Represents quantity in the smallest unit (e.g. shares, contracts)
 * Scale factor of 10000 means a value of 12345 represents 1.2345 shares
 */
class Quantity {
public:
    static constexpr int64_t SCALE_FACTOR = 10000;
    static constexpr Quantity INVALID = Quantity(std::numeric_limits<int64_t>::lowest());
    static constexpr Quantity MAX_VALUE = Quantity(std::numeric_limits<int64_t>::max());
    static constexpr Quantity MIN_VALUE = Quantity(std::numeric_limits<int64_t>::min());
    static constexpr Quantity ZERO = Quantity(0);

    // Default constructor
    constexpr Quantity() : value_(0) {}
    
    // Integer constructor
    constexpr explicit Quantity(int64_t value) : value_(value) {}
    
    // Double constructor - converts to fixed point
    explicit Quantity(double value) : value_(static_cast<int64_t>(value * SCALE_FACTOR)) {}
    
    // Convert to double
    double to_double() const { return static_cast<double>(value_) / SCALE_FACTOR; }
    
    // Get raw fixed point value
    constexpr int64_t raw_value() const { return value_; }
    
    // Arithmetic operators
    Quantity operator+(const Quantity& other) const { return Quantity(value_ + other.value_); }
    Quantity operator-(const Quantity& other) const { return Quantity(value_ - other.value_); }
    Quantity operator*(int64_t multiplier) const { return Quantity(value_ * multiplier); }
    Quantity operator/(int64_t divisor) const { return Quantity(value_ / divisor); }
    
    // Comparison operators
    bool operator==(const Quantity& other) const { return value_ == other.value_; }
    bool operator!=(const Quantity& other) const { return value_ != other.value_; }
    bool operator<(const Quantity& other) const { return value_ < other.value_; }
    bool operator<=(const Quantity& other) const { return value_ <= other.value_; }
    bool operator>(const Quantity& other) const { return value_ > other.value_; }
    bool operator>=(const Quantity& other) const { return value_ >= other.value_; }
    
    // Check if quantity is zero
    bool is_zero() const { return value_ == 0; }
    
    // Output as a string with proper decimal places
    std::string to_string() const;

private:
    int64_t value_;  // Raw fixed point value
};

/**
 * Order Side - buy or sell
 */
enum class Side : uint8_t {
    BUY = 0,
    SELL = 1
};

/**
 * String representation of Side
 */
constexpr const char* to_string(Side side) {
    return side == Side::BUY ? "BUY" : "SELL";
}

/**
 * Order Type - different types of orders
 */
enum class OrderType : uint8_t {
    LIMIT = 0,       // Limit order - specifies price and quantity
    MARKET = 1,      // Market order - specifies only quantity, executes at market price
    CANCEL = 2,      // Cancel order - cancels an existing order
    MODIFY = 3       // Modify order - modifies an existing order
};

/**
 * String representation of OrderType
 */
constexpr const char* to_string(OrderType type) {
    switch (type) {
        case OrderType::LIMIT:  return "LIMIT";
        case OrderType::MARKET: return "MARKET";
        case OrderType::CANCEL: return "CANCEL";
        case OrderType::MODIFY: return "MODIFY";
        default:                return "UNKNOWN";
    }
}

/**
 * Time in Force - how long an order remains active
 */
enum class TimeInForce : uint8_t {
    GTC = 0,     // Good Till Cancel
    IOC = 1,     // Immediate Or Cancel
    FOK = 2      // Fill Or Kill
};

/**
 * String representation of TimeInForce
 */
constexpr const char* to_string(TimeInForce tif) {
    switch (tif) {
        case TimeInForce::GTC: return "GTC";
        case TimeInForce::IOC: return "IOC";
        case TimeInForce::FOK: return "FOK";
        default:               return "UNKNOWN";
    }
}

/**
 * Order Status - current state of an order
 */
enum class OrderStatus : uint8_t {
    NEW = 0,         // New order, not yet processed
    ACCEPTED = 1,    // Order accepted by the system
    REJECTED = 2,    // Order rejected
    FILLED = 3,      // Order completely filled
    PARTIALLY_FILLED = 4, // Order partially filled
    CANCELLED = 5,   // Order cancelled
    REPLACED = 6     // Order replaced/modified
};

/**
 * String representation of OrderStatus
 */
constexpr const char* to_string(OrderStatus status) {
    switch (status) {
        case OrderStatus::NEW:             return "NEW";
        case OrderStatus::ACCEPTED:        return "ACCEPTED";
        case OrderStatus::REJECTED:        return "REJECTED";
        case OrderStatus::FILLED:          return "FILLED";
        case OrderStatus::PARTIALLY_FILLED: return "PARTIALLY_FILLED";
        case OrderStatus::CANCELLED:       return "CANCELLED";
        case OrderStatus::REPLACED:        return "REPLACED";
        default:                           return "UNKNOWN";
    }
}

/**
 * Timestamp - high-resolution timestamp
 * Uses std::chrono::nanoseconds since epoch
 */
using Timestamp = int64_t;

/**
 * Get current timestamp in nanoseconds
 */
inline Timestamp current_timestamp() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

} // namespace orderbook
} // namespace trading_engine

// Hash functions for custom types
namespace std {
    template<>
    struct hash<trading_engine::orderbook::Price> {
        size_t operator()(const trading_engine::orderbook::Price& price) const {
            return std::hash<int64_t>{}(price.raw_value());
        }
    };

    template<>
    struct hash<trading_engine::orderbook::Quantity> {
        size_t operator()(const trading_engine::orderbook::Quantity& qty) const {
            return std::hash<int64_t>{}(qty.raw_value());
        }
    };
} // namespace std 