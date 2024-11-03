add_compile_options(-std=c++20 -fPIC)


add_library(SaopParser 
    Parser.cpp
    Token.cpp
    Lexer.cpp
    ASTNode.cpp

)



# Include directories
target_include_directories(SaopParser PUBLIC
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link against LLVM and Clang libraries
target_link_libraries(SaopParser PUBLIC
    LLVM
    clangBasic

    # Add other Clang libraries if necessary
)
