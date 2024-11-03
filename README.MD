# SimpleAOP: Aspect-Oriented Programming with Clang Plugins

Welcome to **SimpleAOP**, a lightweight Aspect-Oriented Programming (AOP) framework built on top of Clang plugins. This tutorial will guide you through setting up, building, and testing a simple example that demonstrates how to wrap target functions using pointcuts.

---

## Table of Contents

1. [Introduction](#introduction)
2. [Prerequisites](#prerequisites)
3. [Project Structure](#project-structure)
4. [Building the Project](#building-the-project)
5. [Running the Test](#running-the-test)
6. [Understanding the Example](#understanding-the-example)
7. [Defining Pointcuts](#defining-pointcuts)
8. [Using the Plugin Manually](#using-the-plugin-manually)
9. [Extending the Framework](#extending-the-framework)
10. [License](#license)

---

## Introduction

**SimpleAOP** leverages Clang's powerful AST (Abstract Syntax Tree) capabilities to inject additional behavior into existing functions without modifying their source code directly. By defining **pointcuts**, you can specify which functions to wrap and what pre- and post-processing to perform.

In this tutorial, we'll walk through an example where we wrap a simple function `foo` to print messages before and after its execution.

---

## Prerequisites

Before you begin, ensure that you have the following installed on your Linux machine:

- **C++ Compiler**: `clang++` with support for C++20.
- **CMake**: Version 3.13 or higher.
- **Ninja**: Build system generator.
- **LLVM and Clang Libraries**: Development headers and tools.
- **Git**: To clone the repository.

You can install the necessary packages using your distribution's package manager. For example, on Ubuntu:

```bash
sudo apt update
sudo apt install clang llvm libclang-dev cmake ninja-build git
```

---

## Project Structure

Here's an overview of the project's directory structure:

```
SimpleAOP/
├── CMakeLists.txt
├── README.md
├── LICENSE.txt
├── plugin/
│   ├── AST2Matcher.cpp
│   ├── AST2Matcher.h
│   ├── ASTMakeMatcherVisitor.cpp
│   ├── ASTMakeMatcherVisitor.h
│   ├── ASTMatcherP.h
│   ├── CMakeLists.txt
│   ├── Pointer.h
│   ├── SaopPlugin.cpp
│   ├── WrapFunctionCallback.cpp
│   ├── WrapFunctionCallback.h
│   ├── WrapFunctionConsumer.cpp
│   ├── WrapFunctionConsumer.h
│   ├── external_inc/
│   │   └── saop.h
│   └── parser/
│       ├── ASTNode.cpp
│       ├── ASTNode.h
│       ├── CMakeLists.txt
│       ├── Common.h
│       ├── Lexer.cpp
│       ├── Lexer.h
│       ├── Parser.cpp
│       ├── Parser.h
│       ├── Token.cpp
│       ├── Token.def
│       ├── Token.h
│       └── grammar.txt
└── tests/
    ├── CMakeLists.txt
    ├── parser/
    │   ├── CMakeLists.txt
    │   └── ParserMain.cpp
    └── system_test/
        ├── CMakeLists.txt
        ├── pointcut.pc
        ├── saopImpl.h
        └── test.cpp

```

- **plugin/**: Contains the Clang plugin implementation, including AST matchers and consumers.
  - **external_inc/**: Contains external include files, such as `saop.h`.
  - **parser/**: Contains the lexer, parser, and related components for processing pointcuts.
- **tests/**: Contains system tests to verify the functionality.
  - **parser/**: Contains parser-specific tests.
  - **system_test/**: Contains system-level tests, including test executables and configurations.
- **CMakeLists.txt**: Top-level CMake configuration files for building the project.

---

## Building the Project

Follow these steps to build the project:

### 1. Clone the Repository

Clone the SimpleAOP repository from GitHub:

```bash
git clone https://github.com/ormastes/SimpleAOP.git
cd SimpleAOP
```

### 2. Initialize Submodules (If Any)

If the project uses Git submodules, initialize them:

```bash
git submodule update --init --recursive
```

### 3. Create a Build Directory

It's recommended to perform out-of-source builds:

```bash
mkdir build
cd build
```

### 4. Configure the Project with CMake

Run CMake to configure the project and generate build files for Ninja:

```bash
cmake -G Ninja ..
```

### 5. Build All Targets

Build the entire project, including the plugin and tests:

```bash
ninja
```

This command will compile the Clang plugin, the `Pointcut` implementation, and the test executables.

---

## Running the Test

After successfully building the project, you can run the system test to verify that the function wrapping works as expected.

### 1. Execute the Test Using CTest

From the build directory, run the test using CTest:

```bash
ctest --verbose
```

### 2. Expected Output

Upon running the test, you should see output similar to the following:

```
Test project /path/to/SimpleAOP/build
    Start 1: system_test_wrapped_function
1/1 Test #1: system_test_wrapped_function ...........   Passed    0.10 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =    0.10 sec
```

### 3. Manually Running the Executable

You can also manually run the `test_transformed` executable to observe the wrapping behavior:

```bash
./test_transformed
```

**Sample Output:**

```
Before proceeding in Pointcut::around.
Proceeding with original function logic. foo:42
After proceeding in Pointcut::around.
Test Passed: Wrapped function executed correctly.
```

This output demonstrates that the `foo` function has been successfully wrapped, printing messages before and after its execution.

---

## Understanding the Example

Let's break down the key components of the example to understand how function wrapping is achieved.

### 1. `test.cpp`

This file defines the `foo` function and calls it from the `main` function.

```cpp
#include <iostream>
#include "saopImpl.h"

void foo(int x);

int main() {
    foo(42);
    std::cout << "Test Passed: Wrapped function executed correctly." << std::endl;
    return 0;
}

//[[clang::annotate("wrap")]]
#pragma clang section text="data"
void foo(int x) {
    std::cout << "Proceeding with original function logic. foo:" << x << std::endl;
}
```

- **Annotations:**
  - `[[clang::annotate("wrap")]]`: Marks the `foo` function for wrapping.
  - `#pragma clang section text="data"`: Specifies section information used by the plugin.

### 2. `saopImpl.h` and `saopImpl.cpp`

These files define the `Pointcut` class template, which encapsulates the wrapping logic.

**`saopImpl.h`:**

```cpp
#pragma once
#include "saop.h"
#include <iostream>

enum class PointcutName {funcDecl};

template<typename RT, typename... Args>
template<PointcutName Id> 
RT Pointcut<RT, Args...>::around(Args&&... args) {
    // Do your pre-processing here
    std::cout << "Before proceeding in Pointcut::around." << std::endl;
    if constexpr (!std::is_void_v<RT>) {
        return proceed(std::forward<Args>(args)...);
    } else {
        proceed(std::forward<Args>(args)...);
    }
    //  Do your post-processing here
    std::cout << "After proceeding in Pointcut::around." << std::endl;
}
```

**`saopImpl.cpp`:**

```cpp
#include "saopImpl.h"

// Explicit instantiation for the specific PointcutName
template class Pointcut<void, int>;
```

- **Key Points:**
  - **Pre-Processing:** Prints a message before the original function execution.
  - **Proceeding:** Calls the original function via `proceed`.
  - **Post-Processing:** Prints a message after the original function execution.

### 3. Clang Plugin (`WrapFunctionConsumer.h` and `WrapFunctionConsumer.cpp`)

These files implement the Clang plugin that processes the annotated functions and injects the wrapping logic as defined by the pointcuts.

**Key Responsibilities:**

- **AST Matching:** Identifies functions annotated with `[[clang::annotate("wrap")]]`.
- **Code Transformation:** Modifies the AST to insert calls to the `Pointcut::around` method before and after the original function body.

### 4. `pointcut.pc`

Defines the pointcut rule that specifies which functions to wrap and how.

```plaintext
run_pointcut funcDecl = pragma_clang(text, data) || annotation(wrap);
```

- **Pointcut Name:** `funcDecl`
- **Condition:** Targets functions in the `data` section or annotated with `wrap`.

---

## Defining Pointcuts

Pointcuts are the core of the AOP framework, allowing you to specify which functions to target and how to modify them. The pointcut definitions are written in a custom domain-specific language (DSL) defined by the following grammar.

### Pointcut Grammar

```plaintext
////////////////////////////////////////////////////////////////////////////
// Grammar for Pointcut Declarations

// Top-Level Pointcut List
pointcut_list ::= { comment | pointcut_declaration } // zero or more, at least one enforced by parser

// Pointcut Declarations
pointcut_declaration ::= [ "export" ] pointcut_type pointcut_name "=" pointcut_expr ";"

// Pointcut Types
pointcut_type ::= "run_pointcut" | "call_pointcut"

// Pointcut Names
pointcut_name ::= identifier

// Comments
comment ::= "#" { any_character_except_newline } newline

// Pointcut Expressions
pointcut_expr ::= or_expr

or_expr ::= and_expr { "||" and_expr }

and_expr ::= unary_expr { "&&" unary_expr }

unary_expr ::= "!" unary_expr
             | primary_expr

primary_expr ::= "(" pointcut_expr ")"
               | pointcut_primary

// Pointcut Primary Expressions
pointcut_primary ::= func_expression
                   | pragma_clang_expression
                   | annotation_expression
                   | annotation_analysis_expression

// Function Expression
func_expression ::= "func" "(" identifier ")"

// Pragma Clang Expression
pragma_clang_expression ::= "pragma_clang" "(" pragma_kind "," identifier ")"

// Annotation Expression
annotation_expression ::= "annotation" "(" identifier ")"

// Annotation Analysis Expression
annotation_analysis_expression ::= "annotation_analysis" "(" identifier ")"

// Pragma Kind
pragma_kind ::= "bss" | "data" | "relro" | "rodata" | "text"

// Identifiers and Tokens
identifier ::= [a-zA-Z_][a-zA-Z0-9_]*
```

**Note:** Currently, only `run_pointcut` is implemented in the framework.

### Example Pointcut Definitions

Below are some examples of how to define pointcuts using the above grammar.

#### Example 1: Wrapping Functions with Specific Annotations

```plaintext
run_pointcut funcDecl = annotation(wrap);
```

- **Description:** Targets all functions annotated with `[[clang::annotate("wrap")]]`.

#### Example 2: Wrapping Functions in Specific Sections

```plaintext
run_pointcut dataFunctions = pragma_clang(text, data);
```

- **Description:** Targets functions placed in the `data` section via `#pragma clang section text="data"`.

#### Example 3: Combining Conditions with Logical Operators

```plaintext
run_pointcut targetFunctions = pragma_clang(text, data) || annotation(wrap);
```

- **Description:** Targets functions that are either in the `data` section or annotated with `wrap`.

### Using Pointcuts in `pointcut.pc`

You define your pointcuts in the `pointcut.pc` file, which the plugin reads during compilation.

**Sample `pointcut.pc`:**

```plaintext
# Define a pointcut for functions to be wrapped
run_pointcut funcDecl = pragma_clang(text, data) || annotation(wrap);
```

---

## Using the Plugin Manually

While the build system automates the use of the plugin, you may want to run the plugin manually for debugging or integration purposes. This section provides examples of how to invoke the plugin directly via the command line.

### Prerequisites

Ensure that:

- The `SaopPlugin` shared library has been built (e.g., `libSaopPlugin.so`).
- The `pointcut.pc` file is available with your pointcut definitions.

### Example 1: Generating Object Files

You can compile your source code and apply the plugin to generate object files directly.

```bash
clang++ -c test.cpp \
    -Xclang -load -Xclang path/to/libSaopPlugin.so \
    -Xclang -plugin -Xclang saop-plugin \
    -Xclang -plugin-arg-saop-plugin -Xclang "pointcut=path/to/pointcut.pc" \
    -I include/ \
    -o test.o
```

- **Explanation:**
  - `-c`: Compiles the source file to an object file.
  - `-Xclang -load -Xclang path/to/libSaopPlugin.so`: Loads the plugin.
  - `-Xclang -plugin -Xclang saop-plugin`: Activates the plugin.
  - `-Xclang -plugin-arg-saop-plugin -Xclang "pointcut=path/to/pointcut.pc"`: Passes the pointcut file to the plugin.
  - `-I include/`: Includes the header files needed.

### Example 2: Generating Transformed Source Code

You can generate the transformed source code to inspect the modifications made by the plugin.

```bash
clang++ test.cpp \
    -Xclang -load -Xclang path/to/libSaopPlugin.so \
    -Xclang -plugin -Xclang saop-plugin \
    -Xclang -plugin-arg-saop-plugin -Xclang "pointcut=path/to/pointcut.pc" \
    -I include/ \
    -fsyntax-only \
    -DPLUGIN_OUTPUT \
    > transformed_test.cpp
```

- **Explanation:**
  - `-fsyntax-only`: Checks the code for syntax errors without generating object files.
  - `-DPLUGIN_OUTPUT`: Define a macro to enable plugin output (if needed).
  - Redirects the output to `transformed_test.cpp`.

**Note:** The plugin may need to be modified to output the transformed code to `stdout` or a file.

### Example 3: Using CMake to Automate Plugin Invocation

You can automate the plugin invocation in your `CMakeLists.txt` using custom commands.

**CMake Configuration:**

```cmake
set(POINTCUT_VALUE ${CMAKE_CURRENT_SOURCE_DIR}/pointcut.pc)
set(TRANSFORMED_SRC ${CMAKE_CURRENT_BINARY_DIR}/test_transformed.cpp)
set(TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
set(SAOP_LIB_DIR ${PROJECT_SOURCE_DIR}/include)

add_custom_command(
  OUTPUT ${TRANSFORMED_SRC}
  COMMAND clang++ -Xclang -load -Xclang $<TARGET_FILE:SaopPlugin> \
    -Xclang -plugin -Xclang saop-plugin \
    -Xclang -plugin-arg-saop-plugin -Xclang "pointcut=${POINTCUT_VALUE}" \
    -fsyntax-only -I ${SAOP_LIB_DIR} ${TEST_SRC} > ${TRANSFORMED_SRC}
  DEPENDS ${TEST_SRC} SaopPlugin
  COMMENT "Generating transformed source code"
)
```

- **Explanation:**
  - Uses `add_custom_command` to generate the transformed source code during the build process.
  - Sets up dependencies to ensure the plugin and source files are built in the correct order.

---

## Extending the Framework

You can extend **SimpleAOP** to support additional pointcuts and behaviors.

### 1. Adding New Pointcuts

To add a new pointcut:

1. **Define the Pointcut in `pointcut.pc`:**

   ```plaintext
   run_pointcut newPointcut = pragma_clang(rodata, debug) || annotation(newWrap);
   ```

2. **Update `saopImpl.h`:**

   Add the new pointcut name to the `PointcutName` enum:

   ```cpp
   enum class PointcutName {funcDecl, newPointcut};
   ```

3. **Implement the Pointcut Logic:**

   Modify the `Pointcut::around` method or add new methods as needed to handle the new pointcut.

4. **Annotate Target Functions:**

   In your source files, annotate functions you want to wrap with the new pointcut:

   ```cpp
   //[[clang::annotate("newWrap")]]
   #pragma clang section text="debug"
   void newFunction(int y) {
       std::cout << "Executing newFunction with y: " << y << std::endl;
   }
   ```

### 2. Modifying the Plugin

Ensure that the Clang plugin recognizes and processes the new pointcuts by updating the AST matchers and transformation logic accordingly.

---

## License

This project is licensed under the WTFPL (Do What The Fuck You Want To Public License).

---