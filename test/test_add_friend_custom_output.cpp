// Test file for add-friend feature
#include <iostream>
#include <string>

// Simple class
class SimpleClass {
  friend class SimpleClassTest;
  template<typename T> friend class SimpleClassFriend;
  friend class ::SimpleClassMock;
  friend class testing::internal::TestForSimpleClass;

private:
    int privateData = 42;
public:
    int getPrivateData() const { return privateData; }
};

// Class in a namespace
namespace MyNamespace {
    class NamespacedClass {
  friend class NamespacedClassTest;
  template<typename T> friend class NamespacedClassFriend;
  friend class MyNamespace::NamespacedClassMock;
  friend class testing::internal::TestForNamespacedClass;

    private:
        std::string secret = "hidden";
    public:
        const std::string& getSecret() const { return secret; }
    };
}

// Struct (should also get friends)
struct DataStruct {
  friend class DataStructTest;
  template<typename T> friend class DataStructFriend;
  friend class ::DataStructMock;
  friend class testing::internal::TestForDataStruct;

    int x = 10;
    int y = 20;
};

// Template class
template<typename T>
class TemplateClass {
  friend class TemplateClassTest;
  template<typename T> friend class TemplateClassFriend;
  friend class ::TemplateClassMock;
  friend class testing::internal::TestForTemplateClass;

private:
    T value;
public:
    TemplateClass(T v) : value(v) {}
    T getValue() const { return value; }
};

// Nested classes
class OuterClass {
  friend class OuterClassTest;
  template<typename T> friend class OuterClassFriend;
  friend class ::OuterClassMock;
  friend class testing::internal::TestForOuterClass;

public:
    class InnerClass {
  friend class InnerClassTest;
  template<typename T> friend class InnerClassFriend;
  friend class ::InnerClassMock;
  friend class testing::internal::TestForInnerClass;

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