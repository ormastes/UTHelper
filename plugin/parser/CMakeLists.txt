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
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # For Windows, use llvm_map_components_to_libnames
    llvm_map_components_to_libnames(parser_llvm_libs support core)

    target_link_libraries(SaopParser PUBLIC
        ${parser_llvm_libs}
        /build/llvm-project/llvm/build-x86_64-w64-mingw32-static/lib/libclangBasic.a
    )
else()
    target_link_libraries(SaopParser PUBLIC
        LLVM
        /build/llvm-project/llvm/build/lib/libclangBasic.a
    )
endif()
