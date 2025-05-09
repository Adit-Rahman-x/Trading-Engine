set(NETWORK_SOURCES
    # Placeholder for future implementation
)

add_library(network STATIC ${NETWORK_SOURCES})
target_include_directories(network PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(network PUBLIC core)
target_compile_features(network PUBLIC cxx_std_20) 