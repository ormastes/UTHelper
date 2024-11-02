#include <iostream>
#include "saop.h"

void foo(int x);

int main() {
    foo(42);
    return 0;
}
//[[clang::annotate("wrap")]]
#pragma clang section text="data"
void foo(int x) {
    std::cout << "Original foo: " << x << std::endl;
}


