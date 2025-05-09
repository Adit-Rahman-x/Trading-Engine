#include "orderbook/price_level.hpp"
#include <sstream>
#include <algorithm>

namespace trading_engine {
namespace orderbook {

PriceLevel::PriceLevel(Price price)
    : price_(price), total_quantity_(Quantity::ZERO) {
}

void PriceLevel::add_order(OrderPtr order) {
    if (!order || order->price() != price_) {
        return; // Invalid order or price mismatch
    }
    
    // Add to FIFO queue
    orders_.push_back(order);
    
    // Store iterator for quick lookup
    order_map_[order->id()] = std::prev(orders_.end());
    
    // Update total quantity
    total_quantity_ = total_quantity_ + order->remaining_quantity();
}

bool PriceLevel::remove_order(OrderId order_id) {
    auto it = order_map_.find(order_id);
    if (it == order_map_.end()) {
        return false; // Order not found
    }
    
    // Update total quantity
    OrderPtr order = *(it->second);
    total_quantity_ = total_quantity_ - order->remaining_quantity();
    
    // Remove from FIFO queue and map
    orders_.erase(it->second);
    order_map_.erase(it);
    
    return true;
}

bool PriceLevel::modify_order_quantity(OrderId order_id, Quantity new_quantity) {
    auto it = order_map_.find(order_id);
    if (it == order_map_.end()) {
        return false; // Order not found
    }
    
    OrderPtr order = *(it->second);
    
    // Calculate the difference in quantity
    Quantity old_remaining = order->remaining_quantity();
    
    // Ensure new quantity isn't less than already executed
    if (new_quantity < order->executed_quantity()) {
        return false; // Invalid quantity change
    }
    
    // Update the order
    order->set_quantity(new_quantity);
    
    // Update total quantity for this level
    Quantity new_remaining = order->remaining_quantity();
    total_quantity_ = total_quantity_ - old_remaining + new_remaining;
    
    return true;
}

OrderPtr PriceLevel::get_first_order() const {
    if (orders_.empty()) {
        return nullptr;
    }
    return orders_.front();
}

OrderPtr PriceLevel::get_order(OrderId order_id) const {
    auto it = order_map_.find(order_id);
    if (it == order_map_.end()) {
        return nullptr;  // Order not found
    }
    return *(it->second);
}

std::vector<std::pair<OrderPtr, Quantity>> PriceLevel::execute_quantity(Quantity quantity) {
    std::vector<std::pair<OrderPtr, Quantity>> executed_orders;
    
    if (orders_.empty() || quantity <= Quantity::ZERO) {
        return executed_orders;
    }
    
    Quantity remaining_qty = quantity;
    
    // Process orders FIFO until we've executed the requested quantity
    // or run out of orders at this level
    while (!orders_.empty() && remaining_qty > Quantity::ZERO) {
        OrderPtr order = orders_.front();
        
        // Determine how much of this order to execute
        Quantity order_remaining = order->remaining_quantity();
        Quantity exec_qty = (remaining_qty < order_remaining) ? remaining_qty : order_remaining;
        
        if (exec_qty > Quantity::ZERO) {
            // Execute the order
            order->execute(exec_qty);
            
            // Add to executed orders list
            executed_orders.push_back(std::make_pair(order, exec_qty));
            
            // Update total quantity at this level
            total_quantity_ = total_quantity_ - exec_qty;
            
            // Update remaining qty to execute
            remaining_qty = remaining_qty - exec_qty;
        }
        
        // If order is fully executed, remove it
        if (order->is_filled()) {
            order_map_.erase(order->id());
            orders_.pop_front();
        }
    }
    
    return executed_orders;
}

std::vector<OrderPtr> PriceLevel::get_all_orders() const {
    std::vector<OrderPtr> result;
    result.reserve(orders_.size());
    
    // Copy all orders preserving FIFO order
    for (const auto& order : orders_) {
        result.push_back(order);
    }
    
    return result;
}

std::string PriceLevel::to_string() const {
    std::stringstream ss;
    ss << "PriceLevel[price=" << price_.to_string()
       << ", orders=" << orders_.size()
       << ", quantity=" << total_quantity_.to_string()
       << "]";
    return ss.str();
}

} // namespace orderbook
} // namespace trading_engine 