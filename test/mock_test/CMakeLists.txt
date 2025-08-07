cmake_minimum_required(VERSION 3.5)

# Find Google Test
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# Include directories
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

# Create test executable
add_executable(mock_final_test
    ../test_mock_final_test.cpp
)

# Link against Google Test and Google Mock
target_link_libraries(mock_final_test
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    gmock
    gmock_main
    ${CMAKE_THREAD_LIBS_INIT}
)

# Set C++ standard
set_target_properties(mock_final_test PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Add test
enable_testing()
add_test(NAME MockFinalTest COMMAND mock_final_test)