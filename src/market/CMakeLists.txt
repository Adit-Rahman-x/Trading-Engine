set(MARKET_SOURCES
    # Placeholder for future implementation
)

add_library(market STATIC ${MARKET_SOURCES})
target_include_directories(market PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(market PUBLIC core orderbook)
target_compile_features(market PUBLIC cxx_std_20) 