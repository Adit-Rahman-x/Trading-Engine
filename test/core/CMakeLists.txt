set(CORE_TEST_SOURCES
    timer_test.cpp
    logger_test.cpp
    benchmark_test.cpp
)

# Create test executable
add_executable(core_test ${CORE_TEST_SOURCES})
target_link_libraries(core_test PRIVATE core gtest gtest_main)

# Register the test with CTest
add_test(NAME core_test COMMAND core_test) 