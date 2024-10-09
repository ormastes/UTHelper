#include <iostream>
#include "soap.h"

[[clang::annotate("wrap")]]
void foo(int x) {
    std::cout << "Original foo: " << x << std::endl;
}

int main() {
    foo(42);
    return 0;
}
