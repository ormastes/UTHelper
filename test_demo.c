#include <stdio.h>

// Test with section attribute
#pragma clang section text="data"
void function_with_section(int value) {
    printf("Inside function_with_section: %d\n", value);
}
#pragma clang section text=""

// Test with annotation attribute
__attribute__((annotate("wrap")))
void function_with_annotation(const char* message) {
    printf("Inside function_with_annotation: %s\n", message);
}

// Test with both
#pragma clang section text="data"
__attribute__((annotate("wrap")))
void function_with_both(double x, double y) {
    printf("Inside function_with_both: %.2f + %.2f = %.2f\n", x, y, x + y);
}
#pragma clang section text=""

// Normal function - should NOT be wrapped
void normal_function(void) {
    printf("Inside normal_function - not wrapped\n");
}

int main() {
    printf("=== Testing UTHelper Plugin ===\n\n");
    
    printf("Test 1: Function with section attribute\n");
    function_with_section(42);
    
    printf("\nTest 2: Function with annotation attribute\n");
    function_with_annotation("Hello from annotated function");
    
    printf("\nTest 3: Function with both attributes\n");
    function_with_both(3.14, 2.86);
    
    printf("\nTest 4: Normal function (should not be wrapped)\n");
    normal_function();
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
