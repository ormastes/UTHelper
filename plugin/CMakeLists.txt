# Add the parser subdirectory
add_subdirectory(parser)

add_llvm_library(UTHelperPlugin MODULE
  UTHelperPlugin.cpp
  AST2Matcher.cpp
  ASTMakeMatcherVisitor.cpp
  WrapFunctionCallback.cpp
  WrapFunctionConsumer.cpp
  RemoveFinalConsumer.cpp
  MakeVirtualConsumer.cpp
  AddFriendConsumer.cpp
  CompositeConsumer.cpp
  PLUGIN_TOOL
  clang
)

# Include directories
target_include_directories(UTHelperPlugin PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/parser  # Include parser headers
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/external_inc
)

# Link against the parser library and Clang libraries
target_link_libraries(UTHelperPlugin PRIVATE
    SaopParser
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangParse
)

add_dependencies(UTHelperPlugin SaopParser)