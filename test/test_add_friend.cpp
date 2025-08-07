// Test file for add-friend feature
#include <iostream>
#include <string>

// Simple class
class SimpleClass {
private:
    int privateData = 42;
public:
    int getPrivateData() const { return privateData; }
};

// Class in a namespace
namespace MyNamespace {
    class NamespacedClass {
    private:
        std::string secret = "hidden";
    public:
        const std::string& getSecret() const { return secret; }
    };
}

// Struct (should also get friends)
struct DataStruct {
    int x = 10;
    int y = 20;
};

// Template class
template<typename T>
class TemplateClass {
private:
    T value;
public:
    TemplateClass(T v) : value(v) {}
    T getValue() const { return value; }
};

// Nested classes
class OuterClass {
public:
    class InnerClass {
    private:
        double data = 3.14;
    public:
        double getData() const { return data; }
    };
};

int main() {
    SimpleClass simple;
    std::cout << "SimpleClass private data: " << simple.getPrivateData() << std::endl;
    
    MyNamespace::NamespacedClass namespaced;
    std::cout << "NamespacedClass secret: " << namespaced.getSecret() << std::endl;
    
    DataStruct ds;
    std::cout << "DataStruct: x=" << ds.x << ", y=" << ds.y << std::endl;
    
    TemplateClass<int> tc(100);
    std::cout << "TemplateClass value: " << tc.getValue() << std::endl;
    
    OuterClass::InnerClass inner;
    std::cout << "InnerClass data: " << inner.getData() << std::endl;
    
    return 0;
}