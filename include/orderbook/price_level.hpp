#pragma once

#include "orderbook/types.hpp"
#include "orderbook/order.hpp"
#include <list>
#include <unordered_map>
#include <memory>
#include <string>

namespace trading_engine {
namespace orderbook {

/**
 * PriceLevel - represents a single price level in the order book
 * Contains a FIFO queue of orders at the same price
 */
class PriceLevel {
public:
    // Constructor with price
    explicit PriceLevel(Price price);
    
    // Add an order to this price level (at the end of the queue)
    void add_order(OrderPtr order);
    
    // Remove an order from this price level
    bool remove_order(OrderId order_id);
    
    // Modify an existing order's quantity
    bool modify_order_quantity(OrderId order_id, Quantity new_quantity);
    
    // Get the oldest order at this price level
    OrderPtr get_first_order() const;
    
    // Get an order by ID
    OrderPtr get_order(OrderId order_id) const;
    
    // Execute the oldest order(s) for the given quantity
    // Returns a list of executed orders and their quantities
    std::vector<std::pair<OrderPtr, Quantity>> execute_quantity(Quantity quantity);
    
    // Accessors
    Price price() const { return price_; }
    Quantity total_quantity() const { return total_quantity_; }
    size_t order_count() const { return orders_.size(); }
    bool is_empty() const { return orders_.empty(); }
    
    // Get all orders at this price level (in FIFO order)
    std::vector<OrderPtr> get_all_orders() const;
    
    // String representation for debug/logging
    std::string to_string() const;
    
private:
    Price price_;                      // Price level
    Quantity total_quantity_;          // Total quantity of all orders at this level
    std::list<OrderPtr> orders_;       // Orders in FIFO order
    std::unordered_map<OrderId, std::list<OrderPtr>::iterator> order_map_; // For O(1) lookup
};

// Shared pointer typedefs for convenience
using PriceLevelPtr = std::shared_ptr<PriceLevel>;
using ConstPriceLevelPtr = std::shared_ptr<const PriceLevel>;

} // namespace orderbook
} // namespace trading_engine 