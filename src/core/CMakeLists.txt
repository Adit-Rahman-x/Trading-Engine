set(CORE_SOURCES
    timer.cpp
    logger.cpp
    benchmark.cpp
)

add_library(core STATIC ${CORE_SOURCES})
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(core PUBLIC cxx_std_20) 