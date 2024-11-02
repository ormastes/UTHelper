#pragma once

#include <type_traits>
#include <utility>
#include <functional>

// Pointcut structure to hold function pointer and size
template<typename RT, typename... Args>
class Pointcut {
public:
    // For free functions and static member functions
    RT(*func)(Args...);

    // For non-static member functions
    using MemFuncType = RT(*)(void*, Args...);
    MemFuncType mem_func;
    void* obj_ptr;

    std::size_t func_size;

    // Constructor for free functions and static member functions
    constexpr Pointcut(RT(*f)(Args...), std::size_t size)
        : func(f), mem_func(nullptr), obj_ptr(nullptr), func_size(size) {}

    // Constructor for non-static member functions
    constexpr Pointcut(MemFuncType mf, void* obj, std::size_t size)
        : func(nullptr), mem_func(mf), obj_ptr(obj), func_size(size) {}

    // General 'around' function template
    RT around(Args&&... args) {
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
};

// Function to create a Pointcut for free functions and static member functions
template<typename RT, typename... Args>
constexpr auto createPointcut(RT(*func)(Args...), char* func_start, char* func_end) {
    return Pointcut<RT, Args...>(func, static_cast<std::size_t>(func_end - func_start));
}

// Function to create a Pointcut for non-static member functions
template<typename RT, typename ClassType, typename... Args>
constexpr auto createPointcut(RT(ClassType::*func)(Args...), ClassType* obj, char* func_start, char* func_end) {
    using MemFuncType = RT(*)(void*, Args...);
    // Convert member function pointer to a callable function pointer
    MemFuncType mf = [func](void* obj_ptr, Args... args) -> RT {
        return (static_cast<ClassType*>(obj_ptr)->*func)(std::forward<Args>(args)...);
    };
    return Pointcut<RT, Args...>(mf, obj, static_cast<std::size_t>(func_end - func_start));
}


