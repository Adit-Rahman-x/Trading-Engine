#pragma once

#include "orderbook/types.hpp"
#include "orderbook/order.hpp"
#include "orderbook/price_level.hpp"
#include <map>
#include <unordered_map>
#include <memory>
#include <utility>
#include <vector>
#include <optional>
#include <string>

namespace trading_engine {
namespace orderbook {

/**
 * OrderMatch - represents a match between two orders
 */
struct OrderMatch {
    OrderId maker_order_id;      // The resting order
    OrderId taker_order_id;      // The incoming order
    Price match_price;           // Price at which orders matched
    Quantity match_quantity;     // Quantity matched
    Timestamp timestamp;         // When the match occurred
    
    // Constructor
    OrderMatch(OrderId maker, OrderId taker, Price price, Quantity qty)
        : maker_order_id(maker),
          taker_order_id(taker),
          match_price(price),
          match_quantity(qty),
          timestamp(current_timestamp()) {
    }
    
    // String representation
    std::string to_string() const;
};

/**
 * OrderBook - maintains bid and ask sides and matches orders
 */
class OrderBook {
public:
    // Constructor with symbol
    explicit OrderBook(const Symbol& symbol);
    
    // Add a new order to the book
    std::vector<OrderMatch> add_order(OrderPtr order);
    
    // Cancel an existing order
    bool cancel_order(OrderId order_id);
    
    // Modify an existing order (price or quantity)
    std::vector<OrderMatch> modify_order(OrderId order_id, 
                                         std::optional<Price> new_price, 
                                         std::optional<Quantity> new_quantity);
    
    // Get an order by ID
    OrderPtr get_order(OrderId order_id) const;
    
    // Get the best bid price
    std::optional<Price> best_bid() const;
    
    // Get the best ask price
    std::optional<Price> best_ask() const;
    
    // Get the spread (best_ask - best_bid)
    std::optional<Price> spread() const;
    
    // Get the midpoint price (best_bid + best_ask)/2
    std::optional<Price> midpoint() const;
    
    // Get all orders at a specific price level
    std::vector<OrderPtr> get_orders_at_level(Price price, Side side) const;
    
    // Get the total quantity at a specific price level
    Quantity get_quantity_at_level(Price price, Side side) const;
    
    // Get a sorted list of price levels on the bid side (in descending order)
    std::vector<Price> get_bid_prices() const;
    
    // Get a sorted list of price levels on the ask side (in ascending order)
    std::vector<Price> get_ask_prices() const;
    
    // Get all bids as price -> quantity map (in descending order)
    std::map<Price, Quantity, std::greater<Price>> get_bids() const;
    
    // Get all asks as price -> quantity map (in ascending order)
    std::map<Price, Quantity> get_asks() const;
    
    // Get the total quantity across all bid levels
    Quantity get_total_bid_quantity() const;
    
    // Get the total quantity across all ask levels
    Quantity get_total_ask_quantity() const;
    
    // Get the number of price levels on the bid side
    size_t bid_level_count() const;
    
    // Get the number of price levels on the ask side
    size_t ask_level_count() const;
    
    // Get the total number of orders in the book
    size_t order_count() const;
    
    // Get the symbol this book is for
    Symbol symbol() const { return symbol_; }
    
    // Clear the order book (remove all orders)
    void clear();
    
    // Get a string representation of the order book for debug/logging
    std::string to_string() const;
    
private:
    // Match a market order
    std::vector<OrderMatch> match_market_order(OrderPtr order);
    
    // Match a limit order
    std::vector<OrderMatch> match_limit_order(OrderPtr order);
    
    // Add a limit order to the book (after matching)
    void add_limit_order_to_book(OrderPtr order);
    
    // Remove price level if empty
    void remove_price_level_if_empty(Price price, Side side);
    
    // Process a match between two orders
    OrderMatch create_match(OrderPtr maker, OrderPtr taker, Quantity match_qty);
    
    Symbol symbol_;  // The symbol this order book represents
    
    // Order storage - using maps with appropriate comparators for best bid/ask tracking
    std::map<Price, PriceLevelPtr, std::greater<Price>> bid_levels_; // Descending order
    std::map<Price, PriceLevelPtr> ask_levels_;                      // Ascending order
    
    // Order lookup by ID
    std::unordered_map<OrderId, OrderPtr> orders_;
    
    // Total quantity on each side
    Quantity total_bid_quantity_;
    Quantity total_ask_quantity_;
};

// Shared pointer typedef for convenience
using OrderBookPtr = std::shared_ptr<OrderBook>;

} // namespace orderbook
} // namespace trading_engine 