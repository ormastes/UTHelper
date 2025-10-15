# Add the parser subdirectory
add_subdirectory(parser)

# Determine if we're cross-compiling for Windows
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(IS_WINDOWS TRUE)
    link_directories(/build/llvm-project/llvm/build-x86_64-w64-mingw32-static/lib)
else()
    set(IS_WINDOWS FALSE)
    link_directories(/build/llvm-project/llvm/build/lib)
endif()

# Create the plugin library
if(IS_WINDOWS)
    add_library(UTHelperPlugin SHARED
        UTHelperPlugin.cpp
        AST2Matcher.cpp
        ASTMakeMatcherVisitor.cpp
        WrapFunctionCallback.cpp
        WrapFunctionConsumer.cpp
        UnifiedASTVisitor.cpp
    )
else()
    add_llvm_library(UTHelperPlugin MODULE
        UTHelperPlugin.cpp
        AST2Matcher.cpp
        ASTMakeMatcherVisitor.cpp
        WrapFunctionCallback.cpp
        WrapFunctionConsumer.cpp
        UnifiedASTVisitor.cpp
        PLUGIN_TOOL
        clang
    )
endif()

# Include directories
target_include_directories(UTHelperPlugin PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/external_inc
)

# Link libraries
if(IS_WINDOWS)
    # For Windows static build, we need all component libraries

    # Map LLVM components to library names
    llvm_map_components_to_libnames(llvm_libs
        all
    )

    # Clang libraries in dependency order
    set(CLANG_LIBS
        clangBasic
        clangLex
        clangParse
        clangAST
        clangDynamicASTMatchers
        clangASTMatchers
        clangCrossTU
        clangIndex
        clangSema
        clangCodeGen
        clangAnalysis
        clangEdit
        clangRewrite
        clangRewriteFrontend
        clangSerialization
        clangDriver
        clangFrontend
        clangFrontendTool
        clangSupport
    )

    # Link everything with proper grouping for circular dependencies
    target_link_libraries(UTHelperPlugin PRIVATE
        SaopParser
        -Wl,--start-group
        ${CLANG_LIBS}
        -Wl,--end-group
        ${llvm_libs}
        version
    )

    # Set properties for Windows DLL
    set_target_properties(UTHelperPlugin PROPERTIES
        PREFIX ""
        SUFFIX ".dll"
    )

else()
    # For Linux, use the shared clang-cpp library
    target_link_libraries(UTHelperPlugin PRIVATE
        SaopParser
        /build/llvm-project/llvm/build/lib/libclang-cpp.so
    )
endif()

add_dependencies(UTHelperPlugin SaopParser)
