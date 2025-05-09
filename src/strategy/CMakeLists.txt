set(STRATEGY_SOURCES
    # Placeholder for future implementation
)

add_library(strategy STATIC ${STRATEGY_SOURCES})
target_include_directories(strategy PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(strategy PUBLIC core orderbook market)
target_compile_features(strategy PUBLIC cxx_std_20) 