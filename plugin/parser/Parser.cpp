
#include "Parser.h"


// FunctionDecl CallExpr

// Define NamedMatcher struct
template <typename T>
struct NamedMatcher {
    T matcher;
    std::unique_ptr<std::string> name;
    // store real type of matcher
    
    NamedMatcher(const T &m, std::unique_ptr<std::string> n = nullptr)
        : matcher(m), name(std::move(n)) {}
};




