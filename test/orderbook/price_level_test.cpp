#include <gtest/gtest.h>
#include "orderbook/price_level.hpp"
#include <memory>
#include <vector>

using namespace trading_engine::orderbook;

class PriceLevelTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a price level
        price_level_ = std::make_shared<PriceLevel>(Price(100.0));
        
        // Create some orders
        order1_ = std::make_shared<Order>(
            1001,
            "AAPL",
            Side::BUY,
            OrderType::LIMIT,
            Quantity(10.0),
            Price(100.0)
        );
        
        order2_ = std::make_shared<Order>(
            1002,
            "AAPL",
            Side::BUY,
            OrderType::LIMIT,
            Quantity(5.0),
            Price(100.0)
        );
        
        order3_ = std::make_shared<Order>(
            1003,
            "AAPL",
            Side::BUY,
            OrderType::LIMIT,
            Quantity(7.0),
            Price(100.0)
        );
    }
    
    PriceLevelPtr price_level_;
    OrderPtr order1_;
    OrderPtr order2_;
    OrderPtr order3_;
};

TEST_F(PriceLevelTest, Constructor) {
    EXPECT_EQ(price_level_->price().to_double(), 100.0);
    EXPECT_EQ(price_level_->total_quantity().raw_value(), 0);
    EXPECT_EQ(price_level_->order_count(), 0);
    EXPECT_TRUE(price_level_->is_empty());
}

TEST_F(PriceLevelTest, AddOrder) {
    // Add first order
    price_level_->add_order(order1_);
    
    EXPECT_EQ(price_level_->order_count(), 1);
    EXPECT_EQ(price_level_->total_quantity().to_double(), 10.0);
    EXPECT_FALSE(price_level_->is_empty());
    
    // Add second order
    price_level_->add_order(order2_);
    
    EXPECT_EQ(price_level_->order_count(), 2);
    EXPECT_EQ(price_level_->total_quantity().to_double(), 15.0);
    
    // Add third order
    price_level_->add_order(order3_);
    
    EXPECT_EQ(price_level_->order_count(), 3);
    EXPECT_EQ(price_level_->total_quantity().to_double(), 22.0);
    
    // Try to add order with different price (should be ignored)
    OrderPtr wrong_price_order = std::make_shared<Order>(
        1004,
        "AAPL",
        Side::BUY,
        OrderType::LIMIT,
        Quantity(10.0),
        Price(101.0)
    );
    
    price_level_->add_order(wrong_price_order);
    
    EXPECT_EQ(price_level_->order_count(), 3);
    EXPECT_EQ(price_level_->total_quantity().to_double(), 22.0);
}

TEST_F(PriceLevelTest, GetFirstOrder) {
    // Empty level should return nullptr
    EXPECT_EQ(price_level_->get_first_order(), nullptr);
    
    // Add orders and check FIFO order
    price_level_->add_order(order1_);
    price_level_->add_order(order2_);
    
    EXPECT_EQ(price_level_->get_first_order(), order1_);
    
    // Remove first order and check new first
    price_level_->remove_order(order1_->id());
    
    EXPECT_EQ(price_level_->get_first_order(), order2_);
}

TEST_F(PriceLevelTest, RemoveOrder) {
    // Add orders
    price_level_->add_order(order1_);
    price_level_->add_order(order2_);
    price_level_->add_order(order3_);
    
    EXPECT_EQ(price_level_->order_count(), 3);
    EXPECT_EQ(price_level_->total_quantity().to_double(), 22.0);
    
    // Remove middle order
    bool removed = price_level_->remove_order(order2_->id());
    
    EXPECT_TRUE(removed);
    EXPECT_EQ(price_level_->order_count(), 2);
    EXPECT_EQ(price_level_->total_quantity().to_double(), 17.0);
    
    // Try to remove non-existent order
    removed = price_level_->remove_order(9999);
    
    EXPECT_FALSE(removed);
    EXPECT_EQ(price_level_->order_count(), 2);
    EXPECT_EQ(price_level_->total_quantity().to_double(), 17.0);
    
    // Remove remaining orders
    price_level_->remove_order(order1_->id());
    price_level_->remove_order(order3_->id());
    
    EXPECT_EQ(price_level_->order_count(), 0);
    EXPECT_EQ(price_level_->total_quantity().raw_value(), 0);
    EXPECT_TRUE(price_level_->is_empty());
}

TEST_F(PriceLevelTest, ModifyOrderQuantity) {
    // Add order
    price_level_->add_order(order1_);
    
    EXPECT_EQ(price_level_->total_quantity().to_double(), 10.0);
    
    // Increase quantity
    bool modified = price_level_->modify_order_quantity(order1_->id(), Quantity(15.0));
    
    EXPECT_TRUE(modified);
    EXPECT_EQ(price_level_->total_quantity().to_double(), 15.0);
    EXPECT_EQ(order1_->quantity().to_double(), 15.0);
    
    // Decrease quantity
    modified = price_level_->modify_order_quantity(order1_->id(), Quantity(7.0));
    
    EXPECT_TRUE(modified);
    EXPECT_EQ(price_level_->total_quantity().to_double(), 7.0);
    EXPECT_EQ(order1_->quantity().to_double(), 7.0);
    
    // Try to modify non-existent order
    modified = price_level_->modify_order_quantity(9999, Quantity(5.0));
    
    EXPECT_FALSE(modified);
    EXPECT_EQ(price_level_->total_quantity().to_double(), 7.0);
    
    // Partially execute the order
    order1_->execute(Quantity(2.0));
    
    // Try to set quantity below executed amount (should fail)
    modified = price_level_->modify_order_quantity(order1_->id(), Quantity(1.0));
    
    EXPECT_FALSE(modified);
    EXPECT_EQ(price_level_->total_quantity().to_double(), 5.0); // 7.0 - 2.0 = 5.0
    EXPECT_EQ(order1_->quantity().to_double(), 7.0);
    EXPECT_EQ(order1_->executed_quantity().to_double(), 2.0);
}

TEST_F(PriceLevelTest, GetOrder) {
    // Empty level should return nullptr
    EXPECT_EQ(price_level_->get_order(1001), nullptr);
    
    // Add orders
    price_level_->add_order(order1_);
    price_level_->add_order(order2_);
    
    // Get existing order
    EXPECT_EQ(price_level_->get_order(1001), order1_);
    EXPECT_EQ(price_level_->get_order(1002), order2_);
    
    // Get non-existent order
    EXPECT_EQ(price_level_->get_order(9999), nullptr);
}

TEST_F(PriceLevelTest, ExecuteQuantity) {
    // Add orders
    price_level_->add_order(order1_); // 10 shares
    price_level_->add_order(order2_); // 5 shares
    price_level_->add_order(order3_); // 7 shares
    
    // Execute partial quantity of first order
    auto executed = price_level_->execute_quantity(Quantity(6.0));
    
    EXPECT_EQ(executed.size(), 1);
    EXPECT_EQ(executed[0].first, order1_);
    EXPECT_EQ(executed[0].second.to_double(), 6.0);
    EXPECT_EQ(price_level_->total_quantity().to_double(), 16.0); // 22.0 - 6.0 = 16.0
    EXPECT_EQ(order1_->executed_quantity().to_double(), 6.0);
    EXPECT_EQ(order1_->remaining_quantity().to_double(), 4.0);
    EXPECT_EQ(price_level_->order_count(), 3);
    
    // Execute remaining first order and part of second
    executed = price_level_->execute_quantity(Quantity(7.0));
    
    EXPECT_EQ(executed.size(), 2);
    EXPECT_EQ(executed[0].first, order1_);
    EXPECT_EQ(executed[0].second.to_double(), 4.0);
    EXPECT_EQ(executed[1].first, order2_);
    EXPECT_EQ(executed[1].second.to_double(), 3.0);
    EXPECT_EQ(price_level_->total_quantity().to_double(), 9.0); // 16.0 - 7.0 = 9.0
    EXPECT_EQ(order1_->executed_quantity().to_double(), 10.0);
    EXPECT_EQ(order1_->remaining_quantity().to_double(), 0.0);
    EXPECT_TRUE(order1_->is_filled());
    EXPECT_EQ(order2_->executed_quantity().to_double(), 3.0);
    EXPECT_EQ(order2_->remaining_quantity().to_double(), 2.0);
    EXPECT_EQ(price_level_->order_count(), 2); // First order removed
    
    // Execute all remaining orders
    executed = price_level_->execute_quantity(Quantity(20.0)); // More than available
    
    EXPECT_EQ(executed.size(), 2);
    EXPECT_EQ(executed[0].first, order2_);
    EXPECT_EQ(executed[0].second.to_double(), 2.0);
    EXPECT_EQ(executed[1].first, order3_);
    EXPECT_EQ(executed[1].second.to_double(), 7.0);
    EXPECT_EQ(price_level_->total_quantity().to_double(), 0.0);
    EXPECT_EQ(order2_->executed_quantity().to_double(), 5.0);
    EXPECT_EQ(order2_->remaining_quantity().to_double(), 0.0);
    EXPECT_TRUE(order2_->is_filled());
    EXPECT_EQ(order3_->executed_quantity().to_double(), 7.0);
    EXPECT_EQ(order3_->remaining_quantity().to_double(), 0.0);
    EXPECT_TRUE(order3_->is_filled());
    EXPECT_EQ(price_level_->order_count(), 0);
    EXPECT_TRUE(price_level_->is_empty());
    
    // Execute on empty level
    executed = price_level_->execute_quantity(Quantity(5.0));
    
    EXPECT_EQ(executed.size(), 0);
}

TEST_F(PriceLevelTest, GetAllOrders) {
    // Empty level should return empty vector
    EXPECT_TRUE(price_level_->get_all_orders().empty());
    
    // Add orders
    price_level_->add_order(order1_);
    price_level_->add_order(order2_);
    price_level_->add_order(order3_);
    
    // Get all orders
    std::vector<OrderPtr> orders = price_level_->get_all_orders();
    
    EXPECT_EQ(orders.size(), 3);
    EXPECT_EQ(orders[0], order1_);
    EXPECT_EQ(orders[1], order2_);
    EXPECT_EQ(orders[2], order3_);
}

TEST_F(PriceLevelTest, ToString) {
    // Add orders
    price_level_->add_order(order1_);
    price_level_->add_order(order2_);
    
    std::string level_str = price_level_->to_string();
    
    // Verify the string contains important price level details
    EXPECT_NE(level_str.find("PriceLevel[price=100.0000"), std::string::npos);
    EXPECT_NE(level_str.find("orders=2"), std::string::npos);
    EXPECT_NE(level_str.find("quantity=15.0000"), std::string::npos);
} 