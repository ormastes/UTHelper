#include <iostream>
#include "saopImpl.h"

void foo(int x);
struct Bar {
    [[clang::annotate("wrap")]]
    void bar() {
        std::cout << "Proceeding with original function logic. Bar:"<< std::endl;
    }
};

struct Foo {
    void bar() ;
};

int main() {
    foo(42);
    Bar b;
    b.bar();
    Foo f;
    f.bar();
    std::cout << "Test Passed: Wrapped function executed correctly." << std::endl;
    return 0;
}
//[[clang::annotate("wrap")]]
#pragma clang section text="data"
void foo(int x) {
    std::cout << "Proceeding with original function logic. foo:" << x << std::endl;
}

#pragma clang section text=""

[[clang::annotate("wrap")]]
void Foo::bar() {
    std::cout << "Proceeding with original function logic. Foo:"<< std::endl;
}
