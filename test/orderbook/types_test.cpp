#include <gtest/gtest.h>
#include "orderbook/types.hpp"
#include <limits>
#include <sstream>

using namespace trading_engine::orderbook;

TEST(PriceTest, BasicOperations) {
    // Test constructors
    Price p1;  // Default constructor
    EXPECT_EQ(p1.raw_value(), 0);
    
    Price p2(1234);  // Integer constructor
    EXPECT_EQ(p2.raw_value(), 1234);
    
    Price p3(1.2345);  // Double constructor
    EXPECT_EQ(p3.raw_value(), 12345);
    
    // Test conversion to double
    EXPECT_DOUBLE_EQ(p3.to_double(), 1.2345);
    
    // Test arithmetic operators
    Price p4 = p3 + Price(0.1);
    EXPECT_DOUBLE_EQ(p4.to_double(), 1.3345);
    
    Price p5 = p3 - Price(0.1);
    EXPECT_DOUBLE_EQ(p5.to_double(), 1.1345);
    
    Price p6 = p3 * 2;
    EXPECT_DOUBLE_EQ(p6.to_double(), 2.469);
    
    Price p7 = p3 / 2;
    EXPECT_DOUBLE_EQ(p7.to_double(), 0.61725);
    
    // Test comparison operators
    EXPECT_TRUE(p3 == Price(1.2345));
    EXPECT_FALSE(p3 == Price(1.2346));
    
    EXPECT_TRUE(p3 != Price(1.2346));
    EXPECT_FALSE(p3 != Price(1.2345));
    
    EXPECT_TRUE(p3 < Price(1.2346));
    EXPECT_FALSE(p3 < Price(1.2345));
    
    EXPECT_TRUE(p3 <= Price(1.2345));
    EXPECT_TRUE(p3 <= Price(1.2346));
    EXPECT_FALSE(p3 <= Price(1.2344));
    
    EXPECT_TRUE(p3 > Price(1.2344));
    EXPECT_FALSE(p3 > Price(1.2345));
    
    EXPECT_TRUE(p3 >= Price(1.2345));
    EXPECT_TRUE(p3 >= Price(1.2344));
    EXPECT_FALSE(p3 >= Price(1.2346));
}

TEST(PriceTest, SpecialValues) {
    // Test special values
    EXPECT_NE(Price::INVALID.raw_value(), 0);
    EXPECT_EQ(Price::ZERO.raw_value(), 0);
    EXPECT_GT(Price::MAX_VALUE.raw_value(), 0);
    EXPECT_LT(Price::MIN_VALUE.raw_value(), 0);
}

TEST(PriceTest, ToString) {
    // Test to_string method
    Price p1(1.2345);
    EXPECT_EQ(p1.to_string(), "1.2345");
    
    Price p2(-1.2345);
    EXPECT_EQ(p2.to_string(), "-1.2345");
    
    Price p3(0);
    EXPECT_EQ(p3.to_string(), "0.0000");
}

TEST(QuantityTest, BasicOperations) {
    // Test constructors
    Quantity q1;  // Default constructor
    EXPECT_EQ(q1.raw_value(), 0);
    
    Quantity q2(1234);  // Integer constructor
    EXPECT_EQ(q2.raw_value(), 1234);
    
    Quantity q3(1.2345);  // Double constructor
    EXPECT_EQ(q3.raw_value(), 12345);
    
    // Test conversion to double
    EXPECT_DOUBLE_EQ(q3.to_double(), 1.2345);
    
    // Test arithmetic operators
    Quantity q4 = q3 + Quantity(0.1);
    EXPECT_DOUBLE_EQ(q4.to_double(), 1.3345);
    
    Quantity q5 = q3 - Quantity(0.1);
    EXPECT_DOUBLE_EQ(q5.to_double(), 1.1345);
    
    Quantity q6 = q3 * 2;
    EXPECT_DOUBLE_EQ(q6.to_double(), 2.469);
    
    Quantity q7 = q3 / 2;
    EXPECT_DOUBLE_EQ(q7.to_double(), 0.61725);
    
    // Test comparison operators
    EXPECT_TRUE(q3 == Quantity(1.2345));
    EXPECT_FALSE(q3 == Quantity(1.2346));
    
    EXPECT_TRUE(q3 != Quantity(1.2346));
    EXPECT_FALSE(q3 != Quantity(1.2345));
    
    EXPECT_TRUE(q3 < Quantity(1.2346));
    EXPECT_FALSE(q3 < Quantity(1.2345));
    
    EXPECT_TRUE(q3 <= Quantity(1.2345));
    EXPECT_TRUE(q3 <= Quantity(1.2346));
    EXPECT_FALSE(q3 <= Quantity(1.2344));
    
    EXPECT_TRUE(q3 > Quantity(1.2344));
    EXPECT_FALSE(q3 > Quantity(1.2345));
    
    EXPECT_TRUE(q3 >= Quantity(1.2345));
    EXPECT_TRUE(q3 >= Quantity(1.2344));
    EXPECT_FALSE(q3 >= Quantity(1.2346));
    
    // Test is_zero
    EXPECT_TRUE(Quantity(0).is_zero());
    EXPECT_FALSE(Quantity(0.0001).is_zero());
    EXPECT_FALSE(Quantity(-0.0001).is_zero());
}

TEST(QuantityTest, SpecialValues) {
    // Test special values
    EXPECT_NE(Quantity::INVALID.raw_value(), 0);
    EXPECT_EQ(Quantity::ZERO.raw_value(), 0);
    EXPECT_GT(Quantity::MAX_VALUE.raw_value(), 0);
    EXPECT_LT(Quantity::MIN_VALUE.raw_value(), 0);
}

TEST(QuantityTest, ToString) {
    // Test to_string method
    Quantity q1(1.2345);
    EXPECT_EQ(q1.to_string(), "1.2345");
    
    Quantity q2(-1.2345);
    EXPECT_EQ(q2.to_string(), "-1.2345");
    
    Quantity q3(0);
    EXPECT_EQ(q3.to_string(), "0.0000");
}

TEST(EnumTest, ToString) {
    // Test Side to_string
    EXPECT_STREQ(to_string(Side::BUY), "BUY");
    EXPECT_STREQ(to_string(Side::SELL), "SELL");
    
    // Test OrderType to_string
    EXPECT_STREQ(to_string(OrderType::LIMIT), "LIMIT");
    EXPECT_STREQ(to_string(OrderType::MARKET), "MARKET");
    EXPECT_STREQ(to_string(OrderType::CANCEL), "CANCEL");
    EXPECT_STREQ(to_string(OrderType::MODIFY), "MODIFY");
    
    // Test TimeInForce to_string
    EXPECT_STREQ(to_string(TimeInForce::GTC), "GTC");
    EXPECT_STREQ(to_string(TimeInForce::IOC), "IOC");
    EXPECT_STREQ(to_string(TimeInForce::FOK), "FOK");
    
    // Test OrderStatus to_string
    EXPECT_STREQ(to_string(OrderStatus::NEW), "NEW");
    EXPECT_STREQ(to_string(OrderStatus::ACCEPTED), "ACCEPTED");
    EXPECT_STREQ(to_string(OrderStatus::REJECTED), "REJECTED");
    EXPECT_STREQ(to_string(OrderStatus::FILLED), "FILLED");
    EXPECT_STREQ(to_string(OrderStatus::PARTIALLY_FILLED), "PARTIALLY_FILLED");
    EXPECT_STREQ(to_string(OrderStatus::CANCELLED), "CANCELLED");
    EXPECT_STREQ(to_string(OrderStatus::REPLACED), "REPLACED");
}

TEST(TimestampTest, CurrentTimestamp) {
    // Test current_timestamp function returns increasing values
    Timestamp t1 = current_timestamp();
    Timestamp t2 = current_timestamp();
    
    EXPECT_LE(t1, t2);
} 