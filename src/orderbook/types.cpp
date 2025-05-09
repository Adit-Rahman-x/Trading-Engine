#include "orderbook/types.hpp"
#include <iomanip>
#include <sstream>
#include <cmath>

namespace trading_engine {
namespace orderbook {

std::string Price::to_string() const {
    // Handle special cases
    if (*this == INVALID) return "INVALID";
    if (*this == MAX_VALUE) return "MAX";
    if (*this == MIN_VALUE) return "MIN";
    
    // Format as fixed-point number with 4 decimal places
    std::stringstream ss;
    
    // Check if negative
    bool negative = value_ < 0;
    int64_t abs_value = negative ? -value_ : value_;
    
    // Integer part
    int64_t int_part = abs_value / SCALE_FACTOR;
    
    // Fractional part
    int64_t frac_part = abs_value % SCALE_FACTOR;
    
    // Format with proper decimal places
    if (negative) ss << "-";
    ss << int_part << ".";
    ss << std::setw(4) << std::setfill('0') << frac_part;
    
    return ss.str();
}

std::string Quantity::to_string() const {
    // Handle special cases
    if (*this == INVALID) return "INVALID";
    if (*this == MAX_VALUE) return "MAX";
    if (*this == MIN_VALUE) return "MIN";
    
    // Format as fixed-point number with 4 decimal places
    std::stringstream ss;
    
    // Check if negative
    bool negative = value_ < 0;
    int64_t abs_value = negative ? -value_ : value_;
    
    // Integer part
    int64_t int_part = abs_value / SCALE_FACTOR;
    
    // Fractional part
    int64_t frac_part = abs_value % SCALE_FACTOR;
    
    // Format with proper decimal places
    if (negative) ss << "-";
    ss << int_part << ".";
    ss << std::setw(4) << std::setfill('0') << frac_part;
    
    return ss.str();
}

} // namespace orderbook
} // namespace trading_engine 