#pragma once
#include "saop.h"

enum class PointcutName {funcDecl};

template<typename RT, typename... Args>
template<PointcutName Id> 
RT Pointcut<RT, Args...>::around(Args&&... args) {
    if constexpr (!std::is_void_v<RT>) {
        RT result;
        if (func) {
            result = std::invoke(func, std::forward<Args>(args)...);
        } else if (mem_func && obj_ptr) {
            result = mem_func(obj_ptr, std::forward<Args>(args)...);
        }
        return result;
    } else {
        if (func) {
            std::invoke(func, std::forward<Args>(args)...);
        } else if (mem_func && obj_ptr) {
            mem_func(obj_ptr, std::forward<Args>(args)...);
        }
    }
}

