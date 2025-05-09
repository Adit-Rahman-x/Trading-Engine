add_subdirectory(core)
add_subdirectory(orderbook)
add_subdirectory(market)
add_subdirectory(strategy)
add_subdirectory(network)

# Main executable
add_executable(trading_engine main.cpp)
target_link_libraries(trading_engine 
    PRIVATE 
    core 
    orderbook 
    market 
    strategy 
    network
)
target_compile_features(trading_engine PRIVATE cxx_std_20) 