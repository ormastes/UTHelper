include(ExternalProject)


# Add the test executable
add_executable(parser_tests
    ParserMain.cpp

)

# Include directories for LLVM and Clang
target_include_directories(parser_tests PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/plugin/parser  # Include the parser headers
)

# Link against the parser library and GoogleTest
target_link_libraries(parser_tests PRIVATE
    SaopParser
)

add_dependencies(parser_tests SaopParser)

# Add CTest-based test cases
#add_test(NAME testSimplePointcut COMMAND parser_tests testSimplePointcut)
#add_test(NAME testComplexPointcut COMMAND parser_tests testComplexPointcut)

# run the tests
add_test(NAME parser_tests COMMAND parser_tests)