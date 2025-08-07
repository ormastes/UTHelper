#include <iostream>

// Test class with final
class FinalClass final {
public:
    void normalMethod() {
        std::cout << "FinalClass::normalMethod()" << std::endl;
    }
    
    virtual void virtualMethod() final {
        std::cout << "FinalClass::virtualMethod()" << std::endl;
    }
};

// Test struct with final
struct FinalStruct final {
    int value = 42;
};

// Base class with virtual methods
class Base {
public:
    virtual void overridable() {
        std::cout << "Base::overridable()" << std::endl;
    }
    
    virtual void finalMethod() final {
        std::cout << "Base::finalMethod()" << std::endl;
    }
};

// Outside base folder - should not be modified
namespace external {
    class ExternalFinalClass final {
        void method() {}
    };
}

int main() {
    FinalClass fc;
    fc.normalMethod();
    fc.virtualMethod();
    
    FinalStruct fs;
    std::cout << "FinalStruct value: " << fs.value << std::endl;
    
    Base b;
    b.overridable();
    b.finalMethod();
    
    return 0;
}