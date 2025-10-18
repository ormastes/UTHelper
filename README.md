# UTHelper - C++ Unit Testing Helper Plugin

**Status:** ✅ Fully Functional | **Last Updated:** October 18, 2024

A Clang plugin that transforms C++ code to improve testability by removing access restrictions and adding testing hooks.

## Table of Contents

- [Features](#features)
- [Quick Start](#quick-start)
- [Building](#building)
- [Usage](#usage)
- [Examples](#examples)
- [Testing Framework Integration](#testing-framework-integration)
- [Project Structure](#project-structure)
- [Testing](#testing)
- [Dependencies](#dependencies)
- [Contributing](#contributing)

---

## Features

The UTHelper plugin provides three main transformations (all enabled by default):

1. **Remove Final** - Removes `final` specifiers from classes and methods, allowing inheritance for mocking
2. **Make Virtual** - Adds `virtual` to methods, enabling polymorphic behavior for testing
3. **Add Friend** - Injects friend declarations to grant test access to private/protected members

### ✅ Verified Features

All features are tested and verified working:

- ✅ Final removal from classes and structs
- ✅ Virtual addition to all methods (public, protected, private)
- ✅ Friend injection with default templates
- ✅ Custom friend templates with `{namespace}` and `{class-name}` placeholders
- ✅ Works with namespaced and global classes
- ✅ Preserves all access specifiers
- ✅ Compatible with Clang 18 on Linux (tested on Ubuntu)

---

## Quick Start

```bash
# Build
mkdir build-linux && cd build-linux
cmake ..
make UTHelperPlugin -j4

# Use
clang++-18 -Xclang -load -Xclang build-linux/plugin/UTHelperPlugin.so \
           -Xclang -plugin -Xclang uthelper \
           -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/project" \
           -fsyntax-only input.cpp > output.cpp
```

---

## Building

### Prerequisites
- LLVM/Clang 18 development libraries
- CMake 3.10 or higher
- C++20 compatible compiler

### Build Steps

```bash
mkdir build-linux
cd build-linux
cmake ..
make UTHelperPlugin -j4
```

The plugin will be built as `build-linux/plugin/UTHelperPlugin.so`

### Build Status
- ✅ Plugin builds successfully: `build-linux/plugin/UTHelperPlugin.so`
- ✅ All tests pass
- ✅ No known build issues

---

## Usage

### Basic Usage (All Features Enabled)

By default, all transformations are applied:

```bash
clang++-18 -Xclang -load -Xclang build-linux/plugin/UTHelperPlugin.so \
           -Xclang -plugin -Xclang uthelper \
           -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/your/project" \
           -fsyntax-only input.cpp > output.cpp
```

**⚠️ Important**: The `base-folder` parameter is **MANDATORY**. It ensures transformations are only applied to your project files, not system headers.

### Disabling Specific Features

Use disable flags to turn off specific transformations:

```bash
# Only remove final (disable other features)
clang++-18 -Xclang -load -Xclang build-linux/plugin/UTHelperPlugin.so \
           -Xclang -plugin -Xclang uthelper \
           -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/project" \
           -Xclang -plugin-arg-uthelper -Xclang disable-make-virtual \
           -Xclang -plugin-arg-uthelper -Xclang disable-add-friend \
           -fsyntax-only input.cpp > output.cpp
```

Available disable flags:
- `disable-remove-final` - Keep `final` specifiers
- `disable-make-virtual` - Don't add `virtual` to methods
- `disable-add-friend` - Don't add friend declarations

### Custom Friend Classes

When friend injection is enabled (default), you can specify custom friend templates:

```bash
clang++-18 -Xclang -load -Xclang build-linux/plugin/UTHelperPlugin.so \
           -Xclang -plugin -Xclang uthelper \
           -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/project" \
           -Xclang -plugin-arg-uthelper -Xclang "custom-friends=class {namespace}::{class-name}Mock;class testing::TestFor{class-name}" \
           -fsyntax-only input.cpp > output.cpp
```

**Template Variables:**
- `{namespace}` - Replaced with the class's namespace (or `::` for global namespace)
- `{class-name}` - Replaced with the class name

**Default Friends** (always added):
- `friend class {ClassName}Test;`
- `template<typename T> friend class {ClassName}Friend;`

### Arguments Reference

#### Mandatory Arguments
- `base-folder=<path>` - Base directory for transformations (REQUIRED)

#### Optional Arguments
- `disable-remove-final` - Keep `final` keywords
- `disable-make-virtual` - Don't add `virtual` to methods
- `disable-add-friend` - Don't inject friend declarations
- `custom-friends=<list>` - Semicolon-separated list of custom friend templates
- `pointcut=<file>` - Switch to pointcut mode for function wrapping (separate feature)

---

## Examples

### Input Code

```cpp
namespace MyApp {
    class DatabaseHandler final {
    public:
        void connect() {}
    private:
        void authenticate() {}
        std::string password;
    };
}
```

### Output Code (All Features Enabled)

```cpp
namespace MyApp {
    class DatabaseHandler {
      friend class DatabaseHandlerTest;
      template<typename T> friend class DatabaseHandlerFriend;

    public:
        virtual void connect() {}
    private:
        virtual void authenticate() {}
        std::string password;
    };
}
```

### What Changed:
1. ✅ Removed `final` from class declaration
2. ✅ Added `virtual` to both `connect()` and `authenticate()` methods
3. ✅ Added friend declarations for `DatabaseHandlerTest` and `DatabaseHandlerFriend<T>`

### With Custom Friends

```bash
# Using custom friend template
clang++-18 -Xclang -load -Xclang build-linux/plugin/UTHelperPlugin.so \
           -Xclang -plugin -Xclang uthelper \
           -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/project" \
           -Xclang -plugin-arg-uthelper -Xclang "custom-friends=class {namespace}::{class-name}Mock;class gmock::Mock<{class-name}>" \
           -fsyntax-only input.cpp > output.cpp
```

**Output:**
```cpp
namespace MyApp {
    class DatabaseHandler {
      friend class DatabaseHandlerTest;
      template<typename T> friend class DatabaseHandlerFriend;
      friend class MyApp::DatabaseHandlerMock;
      friend class gmock::Mock<DatabaseHandler>;

    public:
        virtual void connect() {}
    private:
        virtual void authenticate() {}
        std::string password;
    };
}
```

### Use Cases

#### 1. Maximum Testability (Default)
All features enabled - best for comprehensive mocking and testing:
```bash
clang++-18 -Xclang -load -Xclang UTHelperPlugin.so \
           -Xclang -plugin -Xclang uthelper \
           -Xclang -plugin-arg-uthelper -Xclang "base-folder=$(pwd)" \
           -fsyntax-only input.cpp > output.cpp
```

#### 2. Only Remove Final
When you just want to enable inheritance:
```bash
clang++-18 -Xclang -load -Xclang UTHelperPlugin.so \
           -Xclang -plugin -Xclang uthelper \
           -Xclang -plugin-arg-uthelper -Xclang "base-folder=$(pwd)" \
           -Xclang -plugin-arg-uthelper -Xclang disable-make-virtual \
           -Xclang -plugin-arg-uthelper -Xclang disable-add-friend \
           -fsyntax-only input.cpp > output.cpp
```

#### 3. Virtual + Friends (No Final Removal)
Keep final but enable mocking capabilities:
```bash
clang++-18 -Xclang -load -Xclang UTHelperPlugin.so \
           -Xclang -plugin -Xclang uthelper \
           -Xclang -plugin-arg-uthelper -Xclang "base-folder=$(pwd)" \
           -Xclang -plugin -Xclang disable-remove-final \
           -fsyntax-only input.cpp > output.cpp
```

---

## Testing Framework Integration

### Google Test / Google Mock

```bash
# Prepare code for mocking
clang++-18 -Xclang -load -Xclang UTHelperPlugin.so \
           -Xclang -plugin -Xclang uthelper \
           -Xclang -plugin-arg-uthelper -Xclang "base-folder=$(pwd)/src" \
           -Xclang -plugin-arg-uthelper -Xclang "custom-friends=class {namespace}::{class-name}Mock" \
           -fsyntax-only src/MyClass.cpp > src/MyClass_testable.cpp

# Now create mock in your test
class MyClassMock : public MyClass {
public:
    MOCK_METHOD(void, connect, (), (override));
    MOCK_METHOD(void, authenticate, (), (override));
};
```

---

## Project Structure

```
UTHelper/
├── CMakeLists.txt              # Main build configuration
├── README.md                   # This file
├── TEST_RESULTS.md             # Test verification results
├── LICENSE.txt                 # License information
├── toolchain.cmake             # Build toolchain config
├── CMakePresets.json           # CMake presets
│
├── plugin/                     # Plugin source code
│   ├── UTHelperPlugin.cpp      # Main plugin entry
│   ├── UnifiedASTVisitor.cpp   # AST visitor implementation
│   ├── UnifiedASTVisitor.h
│   ├── AST2Matcher.cpp         # AST matchers
│   ├── ASTMakeMatcherVisitor.cpp
│   ├── WrapFunctionCallback.cpp # Function wrapping
│   ├── WrapFunctionConsumer.cpp
│   ├── CMakeLists.txt
│   └── parser/                 # Pointcut parser
│       ├── Parser.cpp
│       ├── Lexer.cpp
│       ├── ASTNode.cpp
│       └── CMakeLists.txt
│
├── test/                       # Comprehensive test suite
│   ├── test_all_features.cpp   # Complete feature test
│   ├── test_add_friend.cpp     # Friend injection tests
│   ├── test_remove_final.cpp   # Final removal tests
│   ├── test_mock_final.cpp     # Mocking tests
│   ├── CMakeLists.txt          # Test build config
│   ├── system_test/            # System integration tests
│   ├── parser/                 # Parser tests
│   └── parser_unit_test/       # Parser unit tests
│
└── build-linux/                # Build output directory
    └── plugin/
        └── UTHelperPlugin.so   # Built plugin
```

---

## Testing

All features have been tested with:
- Simple classes/structs
- Classes with inheritance
- Final classes/methods
- Namespaced classes
- Template classes
- Multiple access levels
- Custom friend templates

See [TEST_RESULTS.md](TEST_RESULTS.md) for detailed test results.

---

## Dependencies

- LLVM 18 development libraries
- Clang 18
- CMake 3.10+
- C++20 compiler

---

## Recent Updates

### Cleanup (October 18, 2024)

**Removed:**
- Experimental whole-class rewriter (incomplete, had crashes)
- Duplicate test files from root directory
- Old/redundant markdown files

**Updated:**
- README.md - Complete rewrite with accurate documentation
- CMakeLists.txt - Fixed LLVM 18 linking issues
- parser/CMakeLists.txt - Removed unavailable clangBasic dependency

**Verified:**
- All existing features working correctly
- Plugin builds and runs successfully
- Test suite is comprehensive and organized

---

## Known Issues

**None** - All features working as expected.

---

## Future Enhancements

Potential areas for expansion:
- Support for more Clang versions
- Additional transformation options
- Performance optimizations
- Extended pointcut capabilities

---

## Notes

- Plugin outputs transformed code to stdout
- Use `-fsyntax-only` to skip compilation and only perform transformation
- The `base-folder` parameter ensures system headers are not modified
- Custom friends are added in addition to default friends (not as replacement)
- Template variable substitution happens at transformation time

---

## License

See LICENSE.txt for license information.

---

## Contributing

This plugin is part of the UTHelper project for improving C++ testability.

See git history for contributor information.
