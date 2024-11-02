# Add the parser subdirectory
add_subdirectory(parser)

add_llvm_library(SaopPlugin MODULE
  SaopPlugin.cpp
  AST2Matcher.cpp
  ASTMakeMatcherVisitor.cpp

  PLUGIN_TOOL
  clang
)

# Include directories
target_include_directories(SaopPlugin PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/parser  # Include parser headers
)

# Link against the parser library and Clang libraries
target_link_libraries(SaopPlugin PRIVATE
    SaopParser
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangParse
)

add_dependencies(SaopPlugin SaopParser)