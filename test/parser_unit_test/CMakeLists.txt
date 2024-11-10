# Minimum CMake version required
cmake_minimum_required(VERSION 3.8)

# Project name
project(parser_unit_test)

# Add the executable for the unit test
add_executable(${PROJECT_NAME}
    test_parser.cpp
)

# Include directories for GTest/GMock and the code under test
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${GTEST_INCLUDE_DIR}
        ${GMOCK_INCLUDE_DIR}
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/plugin/parser # Adjust this path if necessary
)

# Link the GTest and GMock libraries and the parser library
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${GTEST_MAIN_LIBRARY}
        ${GMOCK_MAIN_LIBRARY}
        ${GTEST_LIBRARY}
        ${GMOCK_LIBRARY}
        SaopParser # The parser library or code under test
)

# Ensure that the parser, GTest, and GMock libraries are built before the unit tests
add_dependencies(${PROJECT_NAME} SaopParser ${GTEST_LIBRARY}  ${GTEST_MAIN_LIBRARY} ${GMOCK_LIBRARY} ${GMOCK_MAIN_LIBRARY})

# Add the test to the CTest framework
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})