#include <gtest/gtest.h>
#include "orderbook/order.hpp"
#include <memory>

using namespace trading_engine::orderbook;

class OrderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a sample order
        order_ = std::make_shared<Order>(
            1001, // id
            "AAPL", // symbol
            Side::BUY, // side
            OrderType::LIMIT, // type
            Quantity(10.0), // quantity
            Price(150.25), // price
            TimeInForce::GTC // time in force
        );
    }
    
    OrderPtr order_;
};

TEST_F(OrderTest, ConstructorAndAccessors) {
    EXPECT_EQ(order_->id(), 1001);
    EXPECT_EQ(order_->symbol(), "AAPL");
    EXPECT_EQ(order_->side(), Side::BUY);
    EXPECT_EQ(order_->type(), OrderType::LIMIT);
    EXPECT_EQ(order_->quantity().to_double(), 10.0);
    EXPECT_EQ(order_->price().to_double(), 150.25);
    EXPECT_EQ(order_->time_in_force(), TimeInForce::GTC);
    EXPECT_EQ(order_->status(), OrderStatus::NEW);
    EXPECT_EQ(order_->executed_quantity().raw_value(), 0);
    EXPECT_EQ(order_->remaining_quantity().raw_value(), order_->quantity().raw_value());
    EXPECT_GT(order_->timestamp(), 0);
    EXPECT_EQ(order_->timestamp(), order_->last_update());
}

TEST_F(OrderTest, DefaultConstructor) {
    Order empty_order;
    
    EXPECT_EQ(empty_order.id(), INVALID_ORDER_ID);
    EXPECT_EQ(empty_order.symbol(), "");
    EXPECT_EQ(empty_order.side(), Side::BUY);
    EXPECT_EQ(empty_order.type(), OrderType::LIMIT);
    EXPECT_EQ(empty_order.quantity().raw_value(), 0);
    EXPECT_EQ(empty_order.price().raw_value(), 0);
    EXPECT_EQ(empty_order.time_in_force(), TimeInForce::GTC);
    EXPECT_EQ(empty_order.status(), OrderStatus::NEW);
    EXPECT_EQ(empty_order.executed_quantity().raw_value(), 0);
    EXPECT_EQ(empty_order.remaining_quantity().raw_value(), 0);
    EXPECT_GT(empty_order.timestamp(), 0);
    EXPECT_EQ(empty_order.timestamp(), empty_order.last_update());
    
    EXPECT_FALSE(empty_order.is_valid());
}

TEST_F(OrderTest, Execute) {
    // Execute half the order
    order_->execute(Quantity(5.0));
    
    EXPECT_EQ(order_->executed_quantity().to_double(), 5.0);
    EXPECT_EQ(order_->remaining_quantity().to_double(), 5.0);
    EXPECT_EQ(order_->status(), OrderStatus::PARTIALLY_FILLED);
    EXPECT_GT(order_->last_update(), order_->timestamp());
    
    // Execute the rest
    order_->execute(Quantity(5.0));
    
    EXPECT_EQ(order_->executed_quantity().to_double(), 10.0);
    EXPECT_EQ(order_->remaining_quantity().to_double(), 0.0);
    EXPECT_EQ(order_->status(), OrderStatus::FILLED);
    
    // Try to execute more (should be ignored)
    order_->execute(Quantity(1.0));
    
    EXPECT_EQ(order_->executed_quantity().to_double(), 10.0);
    EXPECT_EQ(order_->remaining_quantity().to_double(), 0.0);
    EXPECT_EQ(order_->status(), OrderStatus::FILLED);
}

TEST_F(OrderTest, Cancel) {
    EXPECT_TRUE(order_->is_active());
    
    order_->cancel();
    
    EXPECT_FALSE(order_->is_active());
    EXPECT_EQ(order_->status(), OrderStatus::CANCELLED);
    EXPECT_GT(order_->last_update(), order_->timestamp());
    
    // Partially execute then cancel
    OrderPtr order2 = std::make_shared<Order>(
        1002,
        "AAPL",
        Side::BUY,
        OrderType::LIMIT,
        Quantity(10.0),
        Price(150.25)
    );
    
    order2->execute(Quantity(5.0));
    order2->cancel();
    
    EXPECT_FALSE(order2->is_active());
    EXPECT_EQ(order2->status(), OrderStatus::CANCELLED);
    EXPECT_EQ(order2->executed_quantity().to_double(), 5.0);
}

TEST_F(OrderTest, IsActive) {
    // NEW order is active
    EXPECT_TRUE(order_->is_active());
    
    // ACCEPTED order is active
    order_->set_status(OrderStatus::ACCEPTED);
    EXPECT_TRUE(order_->is_active());
    
    // PARTIALLY_FILLED order is active
    order_->execute(Quantity(5.0));
    EXPECT_TRUE(order_->is_active());
    
    // FILLED order is not active
    order_->execute(Quantity(5.0));
    EXPECT_FALSE(order_->is_active());
    
    // CANCELLED order is not active
    OrderPtr order2 = std::make_shared<Order>(
        1002,
        "AAPL",
        Side::BUY,
        OrderType::LIMIT,
        Quantity(10.0),
        Price(150.25)
    );
    
    order2->cancel();
    EXPECT_FALSE(order2->is_active());
    
    // REJECTED order is not active
    OrderPtr order3 = std::make_shared<Order>(
        1003,
        "AAPL",
        Side::BUY,
        OrderType::LIMIT,
        Quantity(10.0),
        Price(150.25)
    );
    
    order3->set_status(OrderStatus::REJECTED);
    EXPECT_FALSE(order3->is_active());
}

TEST_F(OrderTest, IsFilled) {
    // New order is not filled
    EXPECT_FALSE(order_->is_filled());
    
    // Partially executed order is not filled
    order_->execute(Quantity(5.0));
    EXPECT_FALSE(order_->is_filled());
    
    // Fully executed order is filled
    order_->execute(Quantity(5.0));
    EXPECT_TRUE(order_->is_filled());
    
    // Order with FILLED status is filled regardless of quantity
    OrderPtr order2 = std::make_shared<Order>(
        1002,
        "AAPL",
        Side::BUY,
        OrderType::LIMIT,
        Quantity(10.0),
        Price(150.25)
    );
    
    order2->set_status(OrderStatus::FILLED);
    EXPECT_TRUE(order2->is_filled());
}

TEST_F(OrderTest, ToString) {
    std::string order_str = order_->to_string();
    
    // Verify the string contains important order details
    EXPECT_NE(order_str.find("Order[id=1001"), std::string::npos);
    EXPECT_NE(order_str.find("symbol=AAPL"), std::string::npos);
    EXPECT_NE(order_str.find("side=BUY"), std::string::npos);
    EXPECT_NE(order_str.find("type=LIMIT"), std::string::npos);
    EXPECT_NE(order_str.find("qty=10.0000"), std::string::npos);
    EXPECT_NE(order_str.find("price=150.2500"), std::string::npos);
    EXPECT_NE(order_str.find("tif=GTC"), std::string::npos);
    EXPECT_NE(order_str.find("status=NEW"), std::string::npos);
} 