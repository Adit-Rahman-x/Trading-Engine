#include <gtest/gtest.h>
#include "orderbook/order_book.hpp"
#include <memory>
#include <vector>

using namespace trading_engine::orderbook;

class OrderBookTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create an order book
        order_book_ = std::make_shared<OrderBook>("AAPL");
        
        // Create some buy orders
        buy_order1_ = std::make_shared<Order>(
            1001,
            "AAPL",
            Side::BUY,
            OrderType::LIMIT,
            Quantity(10.0),
            Price(100.0)
        );
        
        buy_order2_ = std::make_shared<Order>(
            1002,
            "AAPL",
            Side::BUY,
            OrderType::LIMIT,
            Quantity(5.0),
            Price(99.0)
        );
        
        buy_order3_ = std::make_shared<Order>(
            1003,
            "AAPL",
            Side::BUY,
            OrderType::LIMIT,
            Quantity(7.0),
            Price(98.0)
        );
        
        // Create some sell orders
        sell_order1_ = std::make_shared<Order>(
            2001,
            "AAPL",
            Side::SELL,
            OrderType::LIMIT,
            Quantity(8.0),
            Price(102.0)
        );
        
        sell_order2_ = std::make_shared<Order>(
            2002,
            "AAPL",
            Side::SELL,
            OrderType::LIMIT,
            Quantity(6.0),
            Price(103.0)
        );
        
        sell_order3_ = std::make_shared<Order>(
            2003,
            "AAPL",
            Side::SELL,
            OrderType::LIMIT,
            Quantity(4.0),
            Price(104.0)
        );
        
        // Create market buy and sell orders
        market_buy_ = std::make_shared<Order>(
            3001,
            "AAPL",
            Side::BUY,
            OrderType::MARKET,
            Quantity(10.0),
            Price(0.0)  // Price is ignored for market orders
        );
        
        market_sell_ = std::make_shared<Order>(
            3002,
            "AAPL",
            Side::SELL,
            OrderType::MARKET,
            Quantity(10.0),
            Price(0.0)  // Price is ignored for market orders
        );
    }
    
    OrderBookPtr order_book_;
    OrderPtr buy_order1_, buy_order2_, buy_order3_;
    OrderPtr sell_order1_, sell_order2_, sell_order3_;
    OrderPtr market_buy_, market_sell_;
};

TEST_F(OrderBookTest, Constructor) {
    EXPECT_EQ(order_book_->symbol(), "AAPL");
    EXPECT_EQ(order_book_->order_count(), 0);
    EXPECT_EQ(order_book_->bid_level_count(), 0);
    EXPECT_EQ(order_book_->ask_level_count(), 0);
    EXPECT_EQ(order_book_->get_total_bid_quantity().raw_value(), 0);
    EXPECT_EQ(order_book_->get_total_ask_quantity().raw_value(), 0);
    EXPECT_FALSE(order_book_->best_bid().has_value());
    EXPECT_FALSE(order_book_->best_ask().has_value());
    EXPECT_FALSE(order_book_->spread().has_value());
    EXPECT_FALSE(order_book_->midpoint().has_value());
}

TEST_F(OrderBookTest, AddLimitOrders) {
    // Add buy orders
    order_book_->add_order(buy_order1_);
    order_book_->add_order(buy_order2_);
    order_book_->add_order(buy_order3_);
    
    // Add sell orders
    order_book_->add_order(sell_order1_);
    order_book_->add_order(sell_order2_);
    order_book_->add_order(sell_order3_);
    
    // Check order book state
    EXPECT_EQ(order_book_->order_count(), 6);
    EXPECT_EQ(order_book_->bid_level_count(), 3);
    EXPECT_EQ(order_book_->ask_level_count(), 3);
    EXPECT_EQ(order_book_->get_total_bid_quantity().to_double(), 22.0);
    EXPECT_EQ(order_book_->get_total_ask_quantity().to_double(), 18.0);
    
    // Check best bid/ask
    EXPECT_TRUE(order_book_->best_bid().has_value());
    EXPECT_TRUE(order_book_->best_ask().has_value());
    EXPECT_EQ(order_book_->best_bid().value().to_double(), 100.0);
    EXPECT_EQ(order_book_->best_ask().value().to_double(), 102.0);
    
    // Check spread and midpoint
    EXPECT_TRUE(order_book_->spread().has_value());
    EXPECT_TRUE(order_book_->midpoint().has_value());
    EXPECT_EQ(order_book_->spread().value().to_double(), 2.0);
    EXPECT_EQ(order_book_->midpoint().value().to_double(), 101.0);
    
    // Check price levels
    auto bid_prices = order_book_->get_bid_prices();
    EXPECT_EQ(bid_prices.size(), 3);
    EXPECT_EQ(bid_prices[0].to_double(), 100.0);
    EXPECT_EQ(bid_prices[1].to_double(), 99.0);
    EXPECT_EQ(bid_prices[2].to_double(), 98.0);
    
    auto ask_prices = order_book_->get_ask_prices();
    EXPECT_EQ(ask_prices.size(), 3);
    EXPECT_EQ(ask_prices[0].to_double(), 102.0);
    EXPECT_EQ(ask_prices[1].to_double(), 103.0);
    EXPECT_EQ(ask_prices[2].to_double(), 104.0);
    
    // Check quantities at levels
    EXPECT_EQ(order_book_->get_quantity_at_level(Price(100.0), Side::BUY).to_double(), 10.0);
    EXPECT_EQ(order_book_->get_quantity_at_level(Price(99.0), Side::BUY).to_double(), 5.0);
    EXPECT_EQ(order_book_->get_quantity_at_level(Price(98.0), Side::BUY).to_double(), 7.0);
    
    EXPECT_EQ(order_book_->get_quantity_at_level(Price(102.0), Side::SELL).to_double(), 8.0);
    EXPECT_EQ(order_book_->get_quantity_at_level(Price(103.0), Side::SELL).to_double(), 6.0);
    EXPECT_EQ(order_book_->get_quantity_at_level(Price(104.0), Side::SELL).to_double(), 4.0);
    
    // Check orders at levels
    EXPECT_EQ(order_book_->get_orders_at_level(Price(100.0), Side::BUY).size(), 1);
    EXPECT_EQ(order_book_->get_orders_at_level(Price(100.0), Side::BUY)[0], buy_order1_);
    
    EXPECT_EQ(order_book_->get_orders_at_level(Price(102.0), Side::SELL).size(), 1);
    EXPECT_EQ(order_book_->get_orders_at_level(Price(102.0), Side::SELL)[0], sell_order1_);
    
    // Check get order by ID
    EXPECT_EQ(order_book_->get_order(1001), buy_order1_);
    EXPECT_EQ(order_book_->get_order(2001), sell_order1_);
    EXPECT_EQ(order_book_->get_order(9999), nullptr);
    
    // Get snapshots
    auto bids = order_book_->get_bids();
    auto asks = order_book_->get_asks();
    
    EXPECT_EQ(bids.size(), 3);
    EXPECT_EQ(asks.size(), 3);
    
    EXPECT_EQ(bids[Price(100.0)].to_double(), 10.0);
    EXPECT_EQ(bids[Price(99.0)].to_double(), 5.0);
    EXPECT_EQ(bids[Price(98.0)].to_double(), 7.0);
    
    EXPECT_EQ(asks[Price(102.0)].to_double(), 8.0);
    EXPECT_EQ(asks[Price(103.0)].to_double(), 6.0);
    EXPECT_EQ(asks[Price(104.0)].to_double(), 4.0);
}

TEST_F(OrderBookTest, CancelOrder) {
    // Add orders
    order_book_->add_order(buy_order1_);
    order_book_->add_order(buy_order2_);
    order_book_->add_order(sell_order1_);
    
    EXPECT_EQ(order_book_->order_count(), 3);
    EXPECT_EQ(order_book_->get_total_bid_quantity().to_double(), 15.0);
    EXPECT_EQ(order_book_->get_total_ask_quantity().to_double(), 8.0);
    
    // Cancel a buy order
    bool cancelled = order_book_->cancel_order(buy_order1_->id());
    
    EXPECT_TRUE(cancelled);
    EXPECT_EQ(order_book_->order_count(), 2);
    EXPECT_EQ(order_book_->get_total_bid_quantity().to_double(), 5.0);
    EXPECT_EQ(order_book_->best_bid().value().to_double(), 99.0);
    EXPECT_EQ(order_book_->get_order(buy_order1_->id()), nullptr);
    EXPECT_EQ(buy_order1_->status(), OrderStatus::CANCELLED);
    
    // Cancel a sell order
    cancelled = order_book_->cancel_order(sell_order1_->id());
    
    EXPECT_TRUE(cancelled);
    EXPECT_EQ(order_book_->order_count(), 1);
    EXPECT_EQ(order_book_->get_total_ask_quantity().to_double(), 0.0);
    EXPECT_FALSE(order_book_->best_ask().has_value());
    EXPECT_EQ(order_book_->get_order(sell_order1_->id()), nullptr);
    EXPECT_EQ(sell_order1_->status(), OrderStatus::CANCELLED);
    
    // Try to cancel a non-existent order
    cancelled = order_book_->cancel_order(9999);
    
    EXPECT_FALSE(cancelled);
    EXPECT_EQ(order_book_->order_count(), 1);
}

TEST_F(OrderBookTest, ModifyOrder) {
    // Add orders
    order_book_->add_order(buy_order1_);
    order_book_->add_order(sell_order1_);
    
    // Modify quantity only (decrease)
    auto matches = order_book_->modify_order(
        buy_order1_->id(),
        std::nullopt,  // No change to price
        Quantity(5.0)  // Decrease quantity
    );
    
    EXPECT_TRUE(matches.empty());
    EXPECT_EQ(order_book_->get_total_bid_quantity().to_double(), 5.0);
    EXPECT_EQ(buy_order1_->quantity().to_double(), 5.0);
    EXPECT_EQ(buy_order1_->status(), OrderStatus::REPLACED);
    
    // Modify quantity only (increase)
    matches = order_book_->modify_order(
        buy_order1_->id(),
        std::nullopt,  // No change to price
        Quantity(12.0)  // Increase quantity
    );
    
    EXPECT_TRUE(matches.empty());
    EXPECT_EQ(order_book_->get_total_bid_quantity().to_double(), 12.0);
    EXPECT_EQ(buy_order1_->quantity().to_double(), 12.0);
    
    // Modify price (to non-crossing)
    matches = order_book_->modify_order(
        buy_order1_->id(),
        Price(101.0),  // New price
        std::nullopt   // No change to quantity
    );
    
    EXPECT_TRUE(matches.empty());
    EXPECT_EQ(order_book_->get_total_bid_quantity().to_double(), 12.0);
    EXPECT_EQ(order_book_->best_bid().value().to_double(), 101.0);
    
    // Modify price (to crossing - should result in a match)
    matches = order_book_->modify_order(
        buy_order1_->id(),
        Price(103.0),  // New price (crosses with sell_order1_ at 102.0)
        std::nullopt   // No change to quantity
    );
    
    EXPECT_EQ(matches.size(), 1);
    EXPECT_EQ(matches[0].maker_order_id, sell_order1_->id());
    EXPECT_EQ(matches[0].taker_order_id, buy_order1_->id());
    EXPECT_EQ(matches[0].match_price.to_double(), 102.0);  // Match at sell price
    EXPECT_EQ(matches[0].match_quantity.to_double(), 8.0);  // Full sell_order1_ quantity
    
    EXPECT_EQ(sell_order1_->status(), OrderStatus::FILLED);
    EXPECT_EQ(buy_order1_->status(), OrderStatus::PARTIALLY_FILLED);
    EXPECT_EQ(buy_order1_->executed_quantity().to_double(), 8.0);
    EXPECT_EQ(buy_order1_->remaining_quantity().to_double(), 4.0);
    
    EXPECT_EQ(order_book_->order_count(), 1);  // Only buy_order1_ remains
    EXPECT_EQ(order_book_->bid_level_count(), 1);
    EXPECT_EQ(order_book_->ask_level_count(), 0);
    EXPECT_EQ(order_book_->get_total_bid_quantity().to_double(), 4.0);
}

TEST_F(OrderBookTest, MatchLimitOrders) {
    // Add non-crossing orders
    order_book_->add_order(buy_order1_);  // Buy 10 @ 100.0
    order_book_->add_order(sell_order1_); // Sell 8 @ 102.0
    
    EXPECT_EQ(order_book_->order_count(), 2);
    EXPECT_EQ(order_book_->get_total_bid_quantity().to_double(), 10.0);
    EXPECT_EQ(order_book_->get_total_ask_quantity().to_double(), 8.0);
    
    // Add a crossing buy order
    OrderPtr crossing_buy = std::make_shared<Order>(
        1004,
        "AAPL",
        Side::BUY,
        OrderType::LIMIT,
        Quantity(5.0),
        Price(103.0)  // Crosses with sell_order1_ at 102.0
    );
    
    auto matches = order_book_->add_order(crossing_buy);
    
    EXPECT_EQ(matches.size(), 1);
    EXPECT_EQ(matches[0].maker_order_id, sell_order1_->id());
    EXPECT_EQ(matches[0].taker_order_id, crossing_buy->id());
    EXPECT_EQ(matches[0].match_price.to_double(), 102.0);  // Match at maker price
    EXPECT_EQ(matches[0].match_quantity.to_double(), 5.0);  // Full crossing_buy quantity
    
    EXPECT_EQ(order_book_->order_count(), 2);  // buy_order1_ and partially filled sell_order1_
    EXPECT_EQ(sell_order1_->status(), OrderStatus::PARTIALLY_FILLED);
    EXPECT_EQ(sell_order1_->executed_quantity().to_double(), 5.0);
    EXPECT_EQ(sell_order1_->remaining_quantity().to_double(), 3.0);
    EXPECT_EQ(crossing_buy->status(), OrderStatus::FILLED);
    
    // Add a crossing sell order
    OrderPtr crossing_sell = std::make_shared<Order>(
        2004,
        "AAPL",
        Side::SELL,
        OrderType::LIMIT,
        Quantity(15.0),
        Price(98.0)  // Crosses with buy_order1_ at 100.0
    );
    
    matches = order_book_->add_order(crossing_sell);
    
    EXPECT_EQ(matches.size(), 1);
    EXPECT_EQ(matches[0].maker_order_id, buy_order1_->id());
    EXPECT_EQ(matches[0].taker_order_id, crossing_sell->id());
    EXPECT_EQ(matches[0].match_price.to_double(), 100.0);  // Match at maker price
    EXPECT_EQ(matches[0].match_quantity.to_double(), 10.0); // Full buy_order1_ quantity
    
    EXPECT_EQ(order_book_->order_count(), 2);  // partially filled sell_order1_ and crossing_sell
    EXPECT_EQ(buy_order1_->status(), OrderStatus::FILLED);
    EXPECT_EQ(crossing_sell->status(), OrderStatus::PARTIALLY_FILLED);
    EXPECT_EQ(crossing_sell->executed_quantity().to_double(), 10.0);
    EXPECT_EQ(crossing_sell->remaining_quantity().to_double(), 5.0);
}

TEST_F(OrderBookTest, MatchMarketOrders) {
    // Add limit orders
    order_book_->add_order(buy_order1_);  // Buy 10 @ 100.0
    order_book_->add_order(buy_order2_);  // Buy 5 @ 99.0
    order_book_->add_order(sell_order1_); // Sell 8 @ 102.0
    order_book_->add_order(sell_order2_); // Sell 6 @ 103.0
    
    // Send a market buy order
    auto matches = order_book_->add_order(market_buy_); // Buy 10 @ market
    
    EXPECT_EQ(matches.size(), 2);
    
    // First match against sell_order1_
    EXPECT_EQ(matches[0].maker_order_id, sell_order1_->id());
    EXPECT_EQ(matches[0].taker_order_id, market_buy_->id());
    EXPECT_EQ(matches[0].match_price.to_double(), 102.0);
    EXPECT_EQ(matches[0].match_quantity.to_double(), 8.0);
    
    // Second match against sell_order2_
    EXPECT_EQ(matches[1].maker_order_id, sell_order2_->id());
    EXPECT_EQ(matches[1].taker_order_id, market_buy_->id());
    EXPECT_EQ(matches[1].match_price.to_double(), 103.0);
    EXPECT_EQ(matches[1].match_quantity.to_double(), 2.0);
    
    EXPECT_EQ(sell_order1_->status(), OrderStatus::FILLED);
    EXPECT_EQ(sell_order2_->status(), OrderStatus::PARTIALLY_FILLED);
    EXPECT_EQ(market_buy_->status(), OrderStatus::FILLED);
    
    // Send a market sell order
    matches = order_book_->add_order(market_sell_); // Sell 10 @ market
    
    EXPECT_EQ(matches.size(), 2);
    
    // First match against buy_order1_
    EXPECT_EQ(matches[0].maker_order_id, buy_order1_->id());
    EXPECT_EQ(matches[0].taker_order_id, market_sell_->id());
    EXPECT_EQ(matches[0].match_price.to_double(), 100.0);
    EXPECT_EQ(matches[0].match_quantity.to_double(), 10.0);
    
    // Second match against buy_order2_
    EXPECT_EQ(matches[1].maker_order_id, buy_order2_->id());
    EXPECT_EQ(matches[1].taker_order_id, market_sell_->id());
    EXPECT_EQ(matches[1].match_price.to_double(), 99.0);
    EXPECT_EQ(matches[1].match_quantity.to_double(), 0.0);
    
    EXPECT_EQ(buy_order1_->status(), OrderStatus::FILLED);
    EXPECT_EQ(buy_order2_->status(), OrderStatus::PARTIALLY_FILLED);
    EXPECT_EQ(market_sell_->status(), OrderStatus::FILLED);
}

TEST_F(OrderBookTest, IOCOrders) {
    // Add some limit orders
    order_book_->add_order(buy_order1_);  // Buy 10 @ 100.0
    order_book_->add_order(sell_order1_); // Sell 8 @ 102.0
    
    // Create an IOC order that will partially fill
    OrderPtr ioc_buy = std::make_shared<Order>(
        1005,
        "AAPL",
        Side::BUY,
        OrderType::LIMIT,
        Quantity(10.0),
        Price(103.0),  // Crosses with sell_order1_ at 102.0
        TimeInForce::IOC
    );
    
    auto matches = order_book_->add_order(ioc_buy);
    
    EXPECT_EQ(matches.size(), 1);
    EXPECT_EQ(matches[0].maker_order_id, sell_order1_->id());
    EXPECT_EQ(matches[0].taker_order_id, ioc_buy->id());
    EXPECT_EQ(matches[0].match_price.to_double(), 102.0);
    EXPECT_EQ(matches[0].match_quantity.to_double(), 8.0);
    
    EXPECT_EQ(sell_order1_->status(), OrderStatus::FILLED);
    EXPECT_EQ(ioc_buy->status(), OrderStatus::PARTIALLY_FILLED);
    EXPECT_EQ(ioc_buy->executed_quantity().to_double(), 8.0);
    EXPECT_EQ(ioc_buy->remaining_quantity().to_double(), 2.0);
    
    // IOC should not be added to the book
    EXPECT_EQ(order_book_->order_count(), 1);  // Only buy_order1_
    EXPECT_EQ(order_book_->get_order(ioc_buy->id()), nullptr);
}

TEST_F(OrderBookTest, FOKOrders) {
    // Add some limit orders
    order_book_->add_order(buy_order1_);  // Buy 10 @ 100.0
    order_book_->add_order(sell_order1_); // Sell 8 @ 102.0
    
    // Create an FOK order that will not fully fill
    OrderPtr fok_buy = std::make_shared<Order>(
        1005,
        "AAPL",
        Side::BUY,
        OrderType::LIMIT,
        Quantity(10.0),
        Price(103.0),  // Crosses with sell_order1_ at 102.0, but sell_order1_ only has 8 shares
        TimeInForce::FOK
    );
    
    auto matches = order_book_->add_order(fok_buy);
    
    EXPECT_TRUE(matches.empty());  // No matches, since FOK can't be fully filled
    EXPECT_EQ(fok_buy->status(), OrderStatus::CANCELLED);
    EXPECT_EQ(fok_buy->executed_quantity().raw_value(), 0);
    
    // Create an FOK order that will fully fill
    OrderPtr fok_buy2 = std::make_shared<Order>(
        1006,
        "AAPL",
        Side::BUY,
        OrderType::LIMIT,
        Quantity(7.0),  // Less than sell_order1_ quantity
        Price(103.0),
        TimeInForce::FOK
    );
    
    matches = order_book_->add_order(fok_buy2);
    
    EXPECT_EQ(matches.size(), 1);
    EXPECT_EQ(matches[0].maker_order_id, sell_order1_->id());
    EXPECT_EQ(matches[0].taker_order_id, fok_buy2->id());
    EXPECT_EQ(matches[0].match_price.to_double(), 102.0);
    EXPECT_EQ(matches[0].match_quantity.to_double(), 7.0);
    
    EXPECT_EQ(sell_order1_->status(), OrderStatus::PARTIALLY_FILLED);
    EXPECT_EQ(fok_buy2->status(), OrderStatus::FILLED);
}

TEST_F(OrderBookTest, ClearOrderBook) {
    // Add some orders
    order_book_->add_order(buy_order1_);
    order_book_->add_order(buy_order2_);
    order_book_->add_order(sell_order1_);
    order_book_->add_order(sell_order2_);
    
    EXPECT_EQ(order_book_->order_count(), 4);
    EXPECT_EQ(order_book_->bid_level_count(), 2);
    EXPECT_EQ(order_book_->ask_level_count(), 2);
    
    // Clear the order book
    order_book_->clear();
    
    EXPECT_EQ(order_book_->order_count(), 0);
    EXPECT_EQ(order_book_->bid_level_count(), 0);
    EXPECT_EQ(order_book_->ask_level_count(), 0);
    EXPECT_EQ(order_book_->get_total_bid_quantity().raw_value(), 0);
    EXPECT_EQ(order_book_->get_total_ask_quantity().raw_value(), 0);
    EXPECT_FALSE(order_book_->best_bid().has_value());
    EXPECT_FALSE(order_book_->best_ask().has_value());
}

TEST_F(OrderBookTest, ToString) {
    // Add some orders
    order_book_->add_order(buy_order1_);
    order_book_->add_order(sell_order1_);
    
    std::string book_str = order_book_->to_string();
    
    // Verify the string contains important order book details
    EXPECT_NE(book_str.find("OrderBook[symbol=AAPL"), std::string::npos);
    EXPECT_NE(book_str.find("bids=1"), std::string::npos);
    EXPECT_NE(book_str.find("asks=1"), std::string::npos);
    EXPECT_NE(book_str.find("orders=2"), std::string::npos);
    EXPECT_NE(book_str.find("bid_qty=10.0000"), std::string::npos);
    EXPECT_NE(book_str.find("ask_qty=8.0000"), std::string::npos);
    EXPECT_NE(book_str.find("best_bid=100.0000"), std::string::npos);
    EXPECT_NE(book_str.find("best_ask=102.0000"), std::string::npos);
    EXPECT_NE(book_str.find("spread=2.0000"), std::string::npos);
} 