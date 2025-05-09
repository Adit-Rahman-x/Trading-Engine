set(ORDERBOOK_TEST_SOURCES
    types_test.cpp
    order_test.cpp
    price_level_test.cpp
    order_book_test.cpp
)

# Create test executable
add_executable(orderbook_test ${ORDERBOOK_TEST_SOURCES})
target_link_libraries(orderbook_test PRIVATE orderbook core gtest gtest_main)

# Register the test with CTest
add_test(NAME orderbook_test COMMAND orderbook_test) 