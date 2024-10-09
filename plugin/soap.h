#ifndef SOAP_H
#define SOAP_H

#include <type_traits>
#include <utility>
#include <functional>

// Pointcut structure to hold function pointer and size
template<typename RT, typename... Args>
class Pointcut {
public:
    RT(*func)(Args...);
    std::size_t func_size;

    // Constructor
    constexpr Pointcut(RT(*f)(Args...), std::size_t size)
        : func(f), func_size(size) {}


// General 'around' function template
RT around(Args&&... args) 
{
    if constexpr (!std::is_void_v<RT>) {
        // Pre-processing (if needed)
        auto result = std::invoke(this->func, std::forward<Args>(args)...);
        // Post-processing (if needed)
        return result;
    } else {
        // Pre-processing (if needed)
        std::invoke(this->func, std::forward<Args>(args)...);
        // Post-processing (if needed)
    }
}
};

// Deduction guide for Pointcut
template<typename RT, typename... Args>
Pointcut(RT(*)(Args...), std::size_t) -> Pointcut<RT, Args...>;

// Function to create a Pointcut
template<typename RT, typename... Args>
constexpr auto createPointcut(RT(*func)(Args...), char* func_start, char* func_end) {
    return Pointcut(func, static_cast<std::size_t>(func_end - func_start));
}

// Concept to check if PC is a Pointcut<RT, Args...>
template<typename PC, typename RT, typename... Args>
concept IsPointcut = std::is_same_v<PC, Pointcut<RT, Args...>>;




#endif // SOAP_H
