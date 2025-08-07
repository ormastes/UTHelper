#!/bin/bash

# Script to demonstrate the full workflow of transforming a final class and testing with Google Mock

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$SCRIPT_DIR/.."
BUILD_DIR="$PROJECT_ROOT/build"
PLUGIN_PATH="$BUILD_DIR/plugin/UTHelperPlugin.so"

echo "=== Step 1: Transform the source file ==="
echo "Removing final keywords and making methods virtual..."

clang++ -Xclang -load -Xclang "$PLUGIN_PATH" \
        -Xclang -plugin -Xclang uthelper \
        -Xclang -plugin-arg-uthelper -Xclang remove-final \
        -Xclang -plugin-arg-uthelper -Xclang make-virtual \
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=$SCRIPT_DIR" \
        -fsyntax-only "$SCRIPT_DIR/test_mock_final.cpp" > "$SCRIPT_DIR/test_mock_final_transformed.h"

echo "Transformation complete!"
echo ""

echo "=== Step 2: Build and run the Google Mock test ==="
echo "Building test..."

# Create a temporary build directory for the test
TEST_BUILD_DIR="$SCRIPT_DIR/mock_test_build"
mkdir -p "$TEST_BUILD_DIR"
cd "$TEST_BUILD_DIR"

# Configure and build
cmake "$SCRIPT_DIR/mock_test"
make

echo ""
echo "=== Step 3: Running the test ==="
./mock_final_test

echo ""
echo "=== Test completed successfully! ==="
echo "This demonstrates that:"
echo "1. The 'final' keyword was successfully removed from DatabaseConnection class"
echo "2. The 'final' keyword was successfully removed from Logger's virtual methods"
echo "3. All methods were made virtual, allowing them to be mocked"
echo "4. Google Mock can now create mocks for these previously final classes"