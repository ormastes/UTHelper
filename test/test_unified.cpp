// Test file for unified AST visitor
// Tests each feature individually and in combinations

#include <iostream>
#include <string>

// Test RemoveFinal feature
namespace RemoveFinalTest {
    // Class with final keyword
    class FinalClass final {
    public:
        void method() {}
    };
    
    // Method with final keyword
    class Base {
    public:
        virtual void virtualMethod() final {}
    };
}

// Test MakeVirtual feature
namespace MakeVirtualTest {
    class NonVirtualClass {
    public:
        void normalMethod() {}
        int anotherMethod(int x) { return x * 2; }
        ~NonVirtualClass() {}  // destructor should become virtual
    };
}

// Test AddFriend feature
namespace AddFriendTest {
    class NeedsFriends {
    private:
        int privateData = 42;
    public:
        int getPrivateData() const { return privateData; }
    };
}

// Test all features combined
namespace CombinedTest {
    class CompleteClass final {
    private:
        std::string data;
    public:
        void method1() { data = "test"; }
        virtual void method2() final { std::cout << data << std::endl; }
        std::string getData() const { return data; }
    };
}

int main() {
    RemoveFinalTest::FinalClass fc;
    fc.method();
    
    MakeVirtualTest::NonVirtualClass nvc;
    nvc.normalMethod();
    
    AddFriendTest::NeedsFriends nf;
    std::cout << "Private data: " << nf.getPrivateData() << std::endl;
    
    CombinedTest::CompleteClass cc;
    cc.method1();
    cc.method2();
    
    return 0;
}