set(TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
set(TRANSFORMED_SRC ${CMAKE_CURRENT_BINARY_DIR}/test_transformed.cpp)
set(ASSEMBLY_FILE_ORIGINAL ${CMAKE_CURRENT_BINARY_DIR}/test_original.s)
set(ASSEMBLY_FILE_TRANSFORMED ${CMAKE_CURRENT_BINARY_DIR}/test_transformed.s)
set(OBJECT_FILE_ORIGINAL ${CMAKE_CURRENT_BINARY_DIR}/test_original.o)
set(OBJECT_FILE_TRANSFORMED ${CMAKE_CURRENT_BINARY_DIR}/test_transformed.o)
set(SOAB_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../plugin)
set(CXX_EXTENSIONS -std=c++20)
# Step 1: Generate transformed source code using the plugin
add_custom_command(
  OUTPUT ${TRANSFORMED_SRC}
  COMMAND clang++ ${CXX_EXTENSIONS} -Xclang -load -Xclang $<TARGET_FILE:SaopPlugin> -Xclang -plugin -Xclang saop-plugin -fsyntax-only -I ${SOAB_LIB_DIR} ${TEST_SRC} > ${TRANSFORMED_SRC}
  DEPENDS ${TEST_SRC} SaopPlugin
  COMMENT "Generating transformed source code"
)

add_custom_target(generate_transformed_source
  DEPENDS ${TRANSFORMED_SRC}
)

# Step 2: Generate assembly from the transformed source code
add_custom_command(
  OUTPUT ${ASSEMBLY_FILE_TRANSFORMED}
  COMMAND clang++ ${CXX_EXTENSIONS} -S ${TRANSFORMED_SRC} -I ${SOAB_LIB_DIR} -o ${ASSEMBLY_FILE_TRANSFORMED}
  DEPENDS ${TRANSFORMED_SRC}
  COMMENT "Generating assembly from transformed source code"
)

add_custom_target(generate_assembly_transformed
  DEPENDS ${ASSEMBLY_FILE_TRANSFORMED}
)

# Ensure the transformed assembly is generated after the transformed source code
add_dependencies(generate_assembly_transformed generate_transformed_source)

# Step 3: Compile the transformed source code to produce an object file
add_custom_command(
  OUTPUT ${OBJECT_FILE_TRANSFORMED}
  COMMAND clang++ ${CXX_EXTENSIONS} -c ${TRANSFORMED_SRC} -I ${SOAB_LIB_DIR} -o ${OBJECT_FILE_TRANSFORMED}
  DEPENDS ${TRANSFORMED_SRC}
  COMMENT "Compiling transformed source code to object file"
)

add_custom_target(compile_transformed_to_object
  DEPENDS ${OBJECT_FILE_TRANSFORMED}
)

add_dependencies(compile_transformed_to_object generate_transformed_source)

# Step 4: Generate assembly from the original source code (without the plugin)
add_custom_command(
  OUTPUT ${ASSEMBLY_FILE_ORIGINAL}
  COMMAND clang++ ${CXX_EXTENSIONS} -S ${TEST_SRC} -I ${SOAB_LIB_DIR} -o ${ASSEMBLY_FILE_ORIGINAL}
  DEPENDS ${TEST_SRC}
  COMMENT "Generating assembly from original source code"
)

add_custom_target(generate_assembly_original
  DEPENDS ${ASSEMBLY_FILE_ORIGINAL}
)

# Step 5: Compile the original source code to produce an object file (without the plugin)
add_custom_command(
  OUTPUT ${OBJECT_FILE_ORIGINAL}
  COMMAND clang++ ${CXX_EXTENSIONS} -c ${TEST_SRC} -I ${SOAB_LIB_DIR} -o ${OBJECT_FILE_ORIGINAL}
  DEPENDS ${TEST_SRC}
  COMMENT "Compiling original source code to object file"
)

add_custom_target(compile_original_to_object
  DEPENDS ${OBJECT_FILE_ORIGINAL}
)

# Build binary from the object files and run
add_executable(test_transformed ${TRANSFORMED_SRC})
set_target_properties(test_transformed PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(test_transformed PRIVATE ${SOAB_LIB_DIR})

# run the tests
add_custom_target(run_test_transformed
  COMMAND ./test_transformed
  DEPENDS test_transformed
)


# Build all targets when building 'make all' in the test directory
add_custom_target(all_tests
  DEPENDS generate_assembly_transformed compile_transformed_to_object generate_assembly_original compile_original_to_object
  run_test_transformed
)
