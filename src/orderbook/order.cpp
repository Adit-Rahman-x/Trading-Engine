#include "orderbook/order.hpp"
#include <sstream>

namespace trading_engine {
namespace orderbook {

Order::Order(OrderId id, Symbol symbol, Side side, OrderType type,
             Quantity quantity, Price price, TimeInForce tif)
    : id_(id),
      symbol_(symbol),
      side_(side),
      type_(type),
      quantity_(quantity),
      executed_quantity_(Quantity::ZERO),
      price_(price),
      time_in_force_(tif),
      status_(OrderStatus::NEW),
      timestamp_(current_timestamp()),
      last_update_(timestamp_) {
}

Order::Order()
    : id_(INVALID_ORDER_ID),
      symbol_(""),
      side_(Side::BUY),
      type_(OrderType::LIMIT),
      quantity_(Quantity::ZERO),
      executed_quantity_(Quantity::ZERO),
      price_(Price::ZERO),
      time_in_force_(TimeInForce::GTC),
      status_(OrderStatus::NEW),
      timestamp_(current_timestamp()),
      last_update_(timestamp_) {
}

void Order::execute(Quantity exec_qty) {
    // Ensure we don't execute more than available
    if (exec_qty > remaining_quantity()) {
        exec_qty = remaining_quantity();
    }
    
    executed_quantity_ = executed_quantity_ + exec_qty;
    
    // Update status
    if (executed_quantity_ == quantity_) {
        status_ = OrderStatus::FILLED;
    } else if (executed_quantity_ > Quantity::ZERO) {
        status_ = OrderStatus::PARTIALLY_FILLED;
    }
    
    last_update_ = current_timestamp();
}

void Order::cancel() {
    if (is_active()) {
        status_ = OrderStatus::CANCELLED;
        last_update_ = current_timestamp();
    }
}

bool Order::is_active() const {
    return status_ == OrderStatus::NEW ||
           status_ == OrderStatus::ACCEPTED ||
           status_ == OrderStatus::PARTIALLY_FILLED;
}

std::string Order::to_string() const {
    std::stringstream ss;
    ss << "Order[id=" << id_ 
       << ", symbol=" << symbol_
       << ", side=" << ::trading_engine::orderbook::to_string(side_)
       << ", type=" << ::trading_engine::orderbook::to_string(type_)
       << ", qty=" << quantity_.to_string()
       << ", exec_qty=" << executed_quantity_.to_string()
       << ", price=" << price_.to_string()
       << ", tif=" << ::trading_engine::orderbook::to_string(time_in_force_)
       << ", status=" << ::trading_engine::orderbook::to_string(status_)
       << ", time=" << timestamp_
       << ", last_update=" << last_update_
       << "]";
    return ss.str();
}

} // namespace orderbook
} // namespace trading_engine 