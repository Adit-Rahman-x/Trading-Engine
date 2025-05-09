#include "orderbook/order_book.hpp"
#include <sstream>
#include <algorithm>
#include <iterator>
#include "core/logger.hpp"

namespace trading_engine {
namespace orderbook {

// OrderMatch to_string implementation
std::string OrderMatch::to_string() const {
    std::stringstream ss;
    ss << "Match[maker=" << maker_order_id
       << ", taker=" << taker_order_id
       << ", price=" << match_price.to_string()
       << ", qty=" << match_quantity.to_string()
       << ", time=" << timestamp
       << "]";
    return ss.str();
}

OrderBook::OrderBook(const Symbol& symbol)
    : symbol_(symbol),
      total_bid_quantity_(Quantity::ZERO),
      total_ask_quantity_(Quantity::ZERO) {
}

std::vector<OrderMatch> OrderBook::add_order(OrderPtr order) {
    if (!order || !order->is_valid()) {
        return {}; // Invalid order
    }
    
    // Set order status to accepted
    order->set_status(OrderStatus::ACCEPTED);
    
    // Match order based on its type
    std::vector<OrderMatch> matches;
    if (order->type() == OrderType::MARKET) {
        matches = match_market_order(order);
    } else if (order->type() == OrderType::LIMIT) {
        matches = match_limit_order(order);
        
        // If the order is not fully filled and not IOC, add it to the book
        if (!order->is_filled() && order->time_in_force() != TimeInForce::IOC) {
            add_limit_order_to_book(order);
        }
    }
    
    return matches;
}

bool OrderBook::cancel_order(OrderId order_id) {
    // Find the order
    auto it = orders_.find(order_id);
    if (it == orders_.end()) {
        return false; // Order not found
    }
    
    OrderPtr order = it->second;
    
    // Get the price level
    Price price = order->price();
    Side side = order->side();
    
    // Remove from appropriate price level
    bool removed = false;
    if (side == Side::BUY) {
        auto level_it = bid_levels_.find(price);
        if (level_it != bid_levels_.end()) {
            PriceLevelPtr level = level_it->second;
            removed = level->remove_order(order_id);
            
            if (removed) {
                // Update total quantity
                total_bid_quantity_ = total_bid_quantity_ - order->remaining_quantity();
                
                // Remove empty level
                remove_price_level_if_empty(price, side);
            }
        }
    } else {
        auto level_it = ask_levels_.find(price);
        if (level_it != ask_levels_.end()) {
            PriceLevelPtr level = level_it->second;
            removed = level->remove_order(order_id);
            
            if (removed) {
                // Update total quantity
                total_ask_quantity_ = total_ask_quantity_ - order->remaining_quantity();
                
                // Remove empty level
                remove_price_level_if_empty(price, side);
            }
        }
    }
    
    if (removed) {
        // Mark the order as cancelled
        order->cancel();
        
        // Remove from orders map
        orders_.erase(it);
        
        return true;
    }
    
    return false;
}

std::vector<OrderMatch> OrderBook::modify_order(OrderId order_id, 
                                               std::optional<Price> new_price, 
                                               std::optional<Quantity> new_quantity) {
    // If neither price nor quantity is modified, do nothing
    if (!new_price && !new_quantity) {
        return {};
    }
    
    // Find the order
    auto it = orders_.find(order_id);
    if (it == orders_.end()) {
        return {}; // Order not found
    }
    
    OrderPtr order = it->second;
    
    // If only quantity is modified and it's a decrease, just update the order
    if (!new_price && new_quantity && *new_quantity <= order->quantity()) {
        // Get the price level
        Price price = order->price();
        Side side = order->side();
        
        if (side == Side::BUY) {
            auto level_it = bid_levels_.find(price);
            if (level_it != bid_levels_.end()) {
                PriceLevelPtr level = level_it->second;
                
                // Update quantity
                Quantity old_remaining = order->remaining_quantity();
                level->modify_order_quantity(order_id, *new_quantity);
                Quantity new_remaining = order->remaining_quantity();
                
                // Update total quantity
                total_bid_quantity_ = total_bid_quantity_ - old_remaining + new_remaining;
            }
        } else {
            auto level_it = ask_levels_.find(price);
            if (level_it != ask_levels_.end()) {
                PriceLevelPtr level = level_it->second;
                
                // Update quantity
                Quantity old_remaining = order->remaining_quantity();
                level->modify_order_quantity(order_id, *new_quantity);
                Quantity new_remaining = order->remaining_quantity();
                
                // Update total quantity
                total_ask_quantity_ = total_ask_quantity_ - old_remaining + new_remaining;
            }
        }
        
        // Mark as replaced
        order->set_status(OrderStatus::REPLACED);
        
        return {};
    }
    
    // Otherwise, treat as cancel and replace
    // Cancel the old order
    bool cancelled = cancel_order(order_id);
    if (!cancelled) {
        return {};
    }
    
    // Create a new order with the same properties but new price/quantity
    OrderPtr new_order = std::make_shared<Order>(
        order_id,
        order->symbol(),
        order->side(),
        order->type(),
        new_quantity.value_or(order->quantity()),
        new_price.value_or(order->price()),
        order->time_in_force()
    );
    
    // Add the new order to the book (which will do matching)
    return add_order(new_order);
}

OrderPtr OrderBook::get_order(OrderId order_id) const {
    auto it = orders_.find(order_id);
    if (it == orders_.end()) {
        return nullptr;
    }
    return it->second;
}

std::optional<Price> OrderBook::best_bid() const {
    if (bid_levels_.empty()) {
        return std::nullopt;
    }
    return bid_levels_.begin()->first;
}

std::optional<Price> OrderBook::best_ask() const {
    if (ask_levels_.empty()) {
        return std::nullopt;
    }
    return ask_levels_.begin()->first;
}

std::optional<Price> OrderBook::spread() const {
    auto bb = best_bid();
    auto ba = best_ask();
    
    if (!bb || !ba) {
        return std::nullopt;
    }
    
    return *ba - *bb;
}

std::optional<Price> OrderBook::midpoint() const {
    auto bb = best_bid();
    auto ba = best_ask();
    
    if (!bb || !ba) {
        return std::nullopt;
    }
    
    // Calculate midpoint (average of best bid and ask)
    int64_t sum = bb->raw_value() + ba->raw_value();
    return Price(sum / 2);
}

std::vector<OrderPtr> OrderBook::get_orders_at_level(Price price, Side side) const {
    if (side == Side::BUY) {
        auto it = bid_levels_.find(price);
        if (it != bid_levels_.end()) {
            return it->second->get_all_orders();
        }
    } else {
        auto it = ask_levels_.find(price);
        if (it != ask_levels_.end()) {
            return it->second->get_all_orders();
        }
    }
    
    return {};
}

Quantity OrderBook::get_quantity_at_level(Price price, Side side) const {
    if (side == Side::BUY) {
        auto it = bid_levels_.find(price);
        if (it != bid_levels_.end()) {
            return it->second->total_quantity();
        }
    } else {
        auto it = ask_levels_.find(price);
        if (it != ask_levels_.end()) {
            return it->second->total_quantity();
        }
    }
    
    return Quantity::ZERO;
}

std::vector<Price> OrderBook::get_bid_prices() const {
    std::vector<Price> prices;
    prices.reserve(bid_levels_.size());
    
    for (const auto& [price, _] : bid_levels_) {
        prices.push_back(price);
    }
    
    return prices;
}

std::vector<Price> OrderBook::get_ask_prices() const {
    std::vector<Price> prices;
    prices.reserve(ask_levels_.size());
    
    for (const auto& [price, _] : ask_levels_) {
        prices.push_back(price);
    }
    
    return prices;
}

std::map<Price, Quantity, std::greater<Price>> OrderBook::get_bids() const {
    std::map<Price, Quantity, std::greater<Price>> result;
    
    for (const auto& [price, level] : bid_levels_) {
        result[price] = level->total_quantity();
    }
    
    return result;
}

std::map<Price, Quantity> OrderBook::get_asks() const {
    std::map<Price, Quantity> result;
    
    for (const auto& [price, level] : ask_levels_) {
        result[price] = level->total_quantity();
    }
    
    return result;
}

Quantity OrderBook::get_total_bid_quantity() const {
    return total_bid_quantity_;
}

Quantity OrderBook::get_total_ask_quantity() const {
    return total_ask_quantity_;
}

size_t OrderBook::bid_level_count() const {
    return bid_levels_.size();
}

size_t OrderBook::ask_level_count() const {
    return ask_levels_.size();
}

size_t OrderBook::order_count() const {
    return orders_.size();
}

void OrderBook::clear() {
    bid_levels_.clear();
    ask_levels_.clear();
    orders_.clear();
    total_bid_quantity_ = Quantity::ZERO;
    total_ask_quantity_ = Quantity::ZERO;
}

std::string OrderBook::to_string() const {
    std::stringstream ss;
    
    ss << "OrderBook[symbol=" << symbol_
       << ", bids=" << bid_level_count()
       << ", asks=" << ask_level_count()
       << ", orders=" << order_count()
       << ", bid_qty=" << total_bid_quantity_.to_string()
       << ", ask_qty=" << total_ask_quantity_.to_string();
    
    if (auto bb = best_bid()) {
        ss << ", best_bid=" << bb->to_string();
    } else {
        ss << ", best_bid=none";
    }
    
    if (auto ba = best_ask()) {
        ss << ", best_ask=" << ba->to_string();
    } else {
        ss << ", best_ask=none";
    }
    
    if (auto sp = spread()) {
        ss << ", spread=" << sp->to_string();
    } else {
        ss << ", spread=none";
    }
    
    ss << "]";
    
    return ss.str();
}

std::vector<OrderMatch> OrderBook::match_market_order(OrderPtr order) {
    std::vector<OrderMatch> matches;
    
    if (!order || order->type() != OrderType::MARKET) {
        return matches;
    }
    
    Side side = order->side();
    Quantity remaining_qty = order->remaining_quantity();
    
    // Market orders execute against the opposite side
    if (side == Side::BUY) {
        // Buy order executes against asks
        while (remaining_qty > Quantity::ZERO && !ask_levels_.empty()) {
            auto it = ask_levels_.begin(); // Best (lowest) ask
            PriceLevelPtr level = it->second;
            
            // Execute quantity at this level
            auto executed = level->execute_quantity(remaining_qty);
            
            for (const auto& [maker, exec_qty] : executed) {
                // Create match record
                matches.push_back(create_match(maker, order, exec_qty));
                
                // Update remaining quantity
                remaining_qty = remaining_qty - exec_qty;
                
                // Update total quantity
                total_ask_quantity_ = total_ask_quantity_ - exec_qty;
            }
            
            // Remove level if empty
            if (level->is_empty()) {
                ask_levels_.erase(it);
            }
            
            // Stop if filled or IOC/FOK with partial fill
            if (remaining_qty <= Quantity::ZERO || 
                (order->time_in_force() == TimeInForce::FOK && remaining_qty < order->quantity())) {
                break;
            }
        }
    } else {
        // Sell order executes against bids
        while (remaining_qty > Quantity::ZERO && !bid_levels_.empty()) {
            auto it = bid_levels_.begin(); // Best (highest) bid
            PriceLevelPtr level = it->second;
            
            // Execute quantity at this level
            auto executed = level->execute_quantity(remaining_qty);
            
            for (const auto& [maker, exec_qty] : executed) {
                // Create match record
                matches.push_back(create_match(maker, order, exec_qty));
                
                // Update remaining quantity
                remaining_qty = remaining_qty - exec_qty;
                
                // Update total quantity
                total_bid_quantity_ = total_bid_quantity_ - exec_qty;
            }
            
            // Remove level if empty
            if (level->is_empty()) {
                bid_levels_.erase(it);
            }
            
            // Stop if filled or IOC/FOK with partial fill
            if (remaining_qty <= Quantity::ZERO || 
                (order->time_in_force() == TimeInForce::FOK && remaining_qty < order->quantity())) {
                break;
            }
        }
    }
    
    // Update order's executed quantity
    order->execute(order->quantity() - remaining_qty);
    
    // Handle FOK orders - if not fully matched, cancel all matches
    if (order->time_in_force() == TimeInForce::FOK && remaining_qty > Quantity::ZERO) {
        order->set_status(OrderStatus::CANCELLED);
        return {}; // Return empty matches
    }
    
    return matches;
}

std::vector<OrderMatch> OrderBook::match_limit_order(OrderPtr order) {
    std::vector<OrderMatch> matches;
    
    if (!order || order->type() != OrderType::LIMIT) {
        return matches;
    }
    
    Side side = order->side();
    Price limit_price = order->price();
    Quantity remaining_qty = order->remaining_quantity();
    
    // Limit orders execute against the opposite side up to their limit price
    if (side == Side::BUY) {
        // Buy order executes against asks with price <= limit_price
        while (remaining_qty > Quantity::ZERO && !ask_levels_.empty()) {
            auto it = ask_levels_.begin(); // Best (lowest) ask
            
            // If the best ask price is higher than our limit, we're done matching
            if (it->first > limit_price) {
                break;
            }
            
            PriceLevelPtr level = it->second;
            
            // Execute quantity at this level
            auto executed = level->execute_quantity(remaining_qty);
            
            for (const auto& [maker, exec_qty] : executed) {
                // Create match record
                matches.push_back(create_match(maker, order, exec_qty));
                
                // Update remaining quantity
                remaining_qty = remaining_qty - exec_qty;
                
                // Update total quantity
                total_ask_quantity_ = total_ask_quantity_ - exec_qty;
            }
            
            // Remove level if empty
            if (level->is_empty()) {
                ask_levels_.erase(it);
            }
            
            // Stop if filled or IOC/FOK with partial fill
            if (remaining_qty <= Quantity::ZERO || 
                (order->time_in_force() == TimeInForce::FOK && remaining_qty < order->quantity())) {
                break;
            }
        }
    } else {
        // Sell order executes against bids with price >= limit_price
        while (remaining_qty > Quantity::ZERO && !bid_levels_.empty()) {
            auto it = bid_levels_.begin(); // Best (highest) bid
            
            // If the best bid price is lower than our limit, we're done matching
            if (it->first < limit_price) {
                break;
            }
            
            PriceLevelPtr level = it->second;
            
            // Execute quantity at this level
            auto executed = level->execute_quantity(remaining_qty);
            
            for (const auto& [maker, exec_qty] : executed) {
                // Create match record
                matches.push_back(create_match(maker, order, exec_qty));
                
                // Update remaining quantity
                remaining_qty = remaining_qty - exec_qty;
                
                // Update total quantity
                total_bid_quantity_ = total_bid_quantity_ - exec_qty;
            }
            
            // Remove level if empty
            if (level->is_empty()) {
                bid_levels_.erase(it);
            }
            
            // Stop if filled or IOC/FOK with partial fill
            if (remaining_qty <= Quantity::ZERO || 
                (order->time_in_force() == TimeInForce::FOK && remaining_qty < order->quantity())) {
                break;
            }
        }
    }
    
    // Update order's executed quantity
    order->execute(order->quantity() - remaining_qty);
    
    // Handle FOK orders - if not fully matched, cancel all matches
    if (order->time_in_force() == TimeInForce::FOK && remaining_qty > Quantity::ZERO) {
        order->set_status(OrderStatus::CANCELLED);
        return {}; // Return empty matches
    }
    
    return matches;
}

void OrderBook::add_limit_order_to_book(OrderPtr order) {
    if (!order || order->type() != OrderType::LIMIT || order->is_filled()) {
        return;
    }
    
    // Get order details
    Side side = order->side();
    Price price = order->price();
    
    // Find or create the price level
    PriceLevelPtr level;
    
    if (side == Side::BUY) {
        auto it = bid_levels_.find(price);
        if (it == bid_levels_.end()) {
            level = std::make_shared<PriceLevel>(price);
            bid_levels_[price] = level;
        } else {
            level = it->second;
        }
        
        // Update total quantity
        total_bid_quantity_ = total_bid_quantity_ + order->remaining_quantity();
    } else {
        auto it = ask_levels_.find(price);
        if (it == ask_levels_.end()) {
            level = std::make_shared<PriceLevel>(price);
            ask_levels_[price] = level;
        } else {
            level = it->second;
        }
        
        // Update total quantity
        total_ask_quantity_ = total_ask_quantity_ + order->remaining_quantity();
    }
    
    // Add the order to the level
    level->add_order(order);
    
    // Add to orders map
    orders_[order->id()] = order;
}

void OrderBook::remove_price_level_if_empty(Price price, Side side) {
    if (side == Side::BUY) {
        auto it = bid_levels_.find(price);
        if (it != bid_levels_.end() && it->second->is_empty()) {
            bid_levels_.erase(it);
        }
    } else {
        auto it = ask_levels_.find(price);
        if (it != ask_levels_.end() && it->second->is_empty()) {
            ask_levels_.erase(it);
        }
    }
}

OrderMatch OrderBook::create_match(OrderPtr maker, OrderPtr taker, Quantity match_qty) {
    using core::TE_LOG_DEBUG;
    
    // Create match record
    OrderMatch match(maker->id(), taker->id(), maker->price(), match_qty);
    
    // Log the match
    TE_LOG_DEBUG("Match: %s", match.to_string().c_str());
    
    return match;
}

} // namespace orderbook
} // namespace trading_engine 