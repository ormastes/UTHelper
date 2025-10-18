add_compile_options(-std=c++20)
# Only add -fPIC for non-MSVC compilers
if(NOT MSVC AND NOT (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
    add_compile_options(-fPIC)
endif()


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
llvm_map_components_to_libnames(parser_llvm_libs support core)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Add library search path
    target_link_directories(SaopParser PUBLIC
        /build/llvm-project/llvm/build-x86_64-w64-mingw32-static/lib
    )
else()
    # Add library search path
    target_link_directories(SaopParser PUBLIC
        /build/llvm-project/llvm/build/lib
    )
endif()

target_link_libraries(SaopParser PUBLIC
    ${parser_llvm_libs}
)
