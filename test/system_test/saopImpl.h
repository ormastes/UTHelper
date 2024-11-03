#pragma once
#include "saop.h"
#include <iostream>

enum class PointcutName {funcDecl};

template<typename RT, typename... Args>
template<PointcutName Id> 
RT Pointcut<RT, Args...>::around(Args&&... args) {
    // Do your pre-processing here
    std::cout << "Before proceeding in Pointcut::around." << std::endl;
    if constexpr (!std::is_void_v<RT>) {
        return proceed(std::forward<Args>(args)...);
    } else {
        proceed(std::forward<Args>(args)...);
    }
    //  Do your post-processing here
    std::cout << "After proceeding in Pointcut::around." << std::endl;
}

