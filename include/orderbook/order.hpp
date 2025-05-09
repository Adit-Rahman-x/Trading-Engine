#pragma once

#include "orderbook/types.hpp"
#include <memory>
#include <string>

namespace trading_engine {
namespace orderbook {

/**
 * Order - represents a single order in the order book
 */
class Order {
public:
    // Constructor for a new order
    Order(OrderId id, Symbol symbol, Side side, OrderType type, 
          Quantity quantity, Price price, TimeInForce tif = TimeInForce::GTC);
    
    // Default constructor for empty order
    Order();
    
    // Accessors
    OrderId id() const { return id_; }
    Symbol symbol() const { return symbol_; }
    Side side() const { return side_; }
    OrderType type() const { return type_; }
    Quantity quantity() const { return quantity_; }
    Quantity executed_quantity() const { return executed_quantity_; }
    Quantity remaining_quantity() const { return quantity_ - executed_quantity_; }
    Price price() const { return price_; }
    TimeInForce time_in_force() const { return time_in_force_; }
    OrderStatus status() const { return status_; }
    Timestamp timestamp() const { return timestamp_; }
    Timestamp last_update() const { return last_update_; }
    
    // Assign/modify values
    void set_price(Price price) { price_ = price; }
    void set_quantity(Quantity quantity) { quantity_ = quantity; }
    void set_status(OrderStatus status) { 
        status_ = status; 
        last_update_ = current_timestamp();
    }
    
    // Execute part or all of the order
    void execute(Quantity exec_qty);
    
    // Cancel the order
    void cancel();
    
    // Check if the order is active (can be executed)
    bool is_active() const;
    
    // Check if the order is fully executed
    bool is_filled() const { 
        return executed_quantity_ == quantity_ || status_ == OrderStatus::FILLED; 
    }
    
    // Check if order is valid
    bool is_valid() const { return id_ != INVALID_ORDER_ID; }
    
    // String representation for debug/logging
    std::string to_string() const;
    
private:
    OrderId id_;
    Symbol symbol_;
    Side side_;
    OrderType type_;
    Quantity quantity_;
    Quantity executed_quantity_; // How much of the order has been executed
    Price price_;
    TimeInForce time_in_force_;
    OrderStatus status_;
    Timestamp timestamp_;       // Time when order was created
    Timestamp last_update_;     // Time of last status change
};

// Shared pointer typedefs for convenience
using OrderPtr = std::shared_ptr<Order>;
using ConstOrderPtr = std::shared_ptr<const Order>;

} // namespace orderbook
} // namespace trading_engine 