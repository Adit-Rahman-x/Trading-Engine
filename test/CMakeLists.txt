find_package(GTest QUIET)
if(NOT GTest_FOUND)
  # Download and build googletest
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
  )
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()

add_subdirectory(core)
add_subdirectory(orderbook)
add_subdirectory(market)
add_subdirectory(strategy)
add_subdirectory(network) 