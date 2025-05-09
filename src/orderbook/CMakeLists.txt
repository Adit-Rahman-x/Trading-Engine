set(ORDERBOOK_SOURCES
    types.cpp
    order.cpp
    price_level.cpp
    order_book.cpp
)

add_library(orderbook STATIC ${ORDERBOOK_SOURCES})
target_include_directories(orderbook PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(orderbook PUBLIC core)
target_compile_features(orderbook PUBLIC cxx_std_20) 