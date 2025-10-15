#pragma once
#include <stdio.h>

// Simplified version of saop.h for testing
template<typename RT, typename... Args>
struct Pointcut {
    using proceed_type = RT(*)(Args...);
    
    template<int Id>
    static RT around(Args... args);
    
    static RT proceed(Args... args);
    static proceed_type original;
};

enum class PointcutName { funcDecl };

template<typename RT, typename... Args>
typename Pointcut<RT, Args...>::proceed_type Pointcut<RT, Args...>::original = nullptr;

template<typename RT, typename... Args>
RT Pointcut<RT, Args...>::proceed(Args... args) {
    return original(args...);
}

template<typename RT, typename... Args>
template<int Id>
RT Pointcut<RT, Args...>::around(Args... args) {
    printf("  [BEFORE] Wrapper: Entering wrapped function\n");
    if constexpr (std::is_void_v<RT>) {
        proceed(args...);
        printf("  [AFTER] Wrapper: Exiting wrapped function\n");
    } else {
        RT result = proceed(args...);
        printf("  [AFTER] Wrapper: Exiting wrapped function\n");
        return result;
    }
}
