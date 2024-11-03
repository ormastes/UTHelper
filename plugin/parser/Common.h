#pragma once

// Include standard headers
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <memory> // For std::unique_ptr

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Signals.h" // For llvm::sys::PrintStackTrace
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/Error.h"


#define DEBUG_MODE 1


#if (DEBUG_MODE == 1)
#define DEBUG_PRINT(x) DEBUG_LOG(x)
#define DEBUG_NODE_LOG(x) if (x) { x->print(llvm::errs()); };
#else
#define DEBUG_PRINT(x)
#define DEBUG_NODE_LOG(x)
#endif

#define DEBUG_LOG(x) llvm::errs() << __FILE__ << ":" << __LINE__ << " - " << x << "\n"

// print callstak
#define DUMP_STACK() llvm::sys::PrintStackTrace(llvm::errs())

#define ASSERT(x) if (!(x)) { DEBUG_LOG("Assertion failed: " #x); exit(1); }
#define ASSERT_EQ(x, y) if ((x) != (y)) { DEBUG_LOG("Assertion failed: " #x << ":" <<x <<" == " #y<<":"<<y ); exit(1); }
#define ASSERT_NE(x, y) if ((x) == (y)) { DEBUG_LOG("Assertion failed: " #x << ":" <<x <<" != " #y<<":"<<y ); exit(1); }
#define ASSERT_LT(x, y) if ((x) >= (y)) { DEBUG_LOG("Assertion failed: " #x << ":" <<x <<" < " #y<<":"<<y ); exit(1); }
#define ASSERT_LE(x, y) if ((x) > (y)) { DEBUG_LOG("Assertion failed: " #x << ":" <<x <<" <= " #y<<":"<<y ); exit(1); }
#define ASSERT_GT(x, y) if ((x) <= (y)) { DEBUG_LOG("Assertion failed: " #x << ":" <<x <<" > " #y<<":"<<y ); exit(1); }
#define ASSERT_GE(x, y) if ((x) < (y)) { DEBUG_LOG("Assertion failed: " #x << ":" <<x <<" >= " #y<<":"<<y ); exit(1); }
#define ASSERT_NULL(x) if ((x) != nullptr) { DEBUG_LOG("Assertion failed: " #x << " is not null"); exit(1); }
#define ASSERT_NOT_NULL(x) if ((x) == nullptr) { DEBUG_LOG("Assertion failed: " #x << " is null"); exit(1); }
#define ASSERT_MSG(x, msg) if (!(x)) { DEBUG_LOG(msg); exit(1); }

#define CONTENT_PRINT_SIZE 10
#define CASSERT(x) if (!(x)) { printContext(llvm::errs()); DEBUG_LOG("Assertion failed: " #x << "\n" ); exit(1); }
#define CASSERT_EQ(x, y) if ((x) != (y)) { printContext(llvm::errs());DEBUG_LOG("Assertion failed: " #x << ":" <<x <<" == " #y<<":"<<y << "\n" ); exit(1); }
#define CASSERT_NE(x, y) if ((x) == (y)) { printContext(llvm::errs());DEBUG_LOG("Assertion failed: " #x << ":" <<x <<" != " #y<<":"<<y << "\n" ); exit(1); }
#define CASSERT_LT(x, y) if ((x) >= (y)) { printContext(llvm::errs());DEBUG_LOG("Assertion failed: " #x << ":" <<x <<" < " #y<<":"<<y << "\n" ); exit(1); }
#define CASSERT_LE(x, y) if ((x) > (y)) { printContext(llvm::errs());DEBUG_LOG("Assertion failed: " #x << ":" <<x <<" <= " #y<<":"<<y << "\n" ); exit(1); }
#define CASSERT_GT(x, y) if ((x) <= (y)) { printContext(llvm::errs());DEBUG_LOG("Assertion failed: " #x << ":" <<x <<" > " #y<<":"<<y << "\n" ); exit(1); }
#define CASSERT_GE(x, y) if ((x) < (y)) { printContext(llvm::errs());DEBUG_LOG("Assertion failed: " #x << ":" <<x <<" >= " #y<<":"<<y << "\n" ); exit(1); }
#define CASSERT_NULL(x) if ((x) != nullptr) { printContext(llvm::errs());DEBUG_LOG("Assertion failed: " #x << " is not null" << "\n" ); exit(1); }
#define CASSERT_NOT_NULL(x) if ((x) == nullptr) { printContext(llvm::errs());DEBUG_LOG("Assertion failed: " #x << " is null" << "\n" ); exit(1); }
#define CASSERT_MSG(x, msg) if (!(x)) { printContext(llvm::errs());DEBUG_LOG(msg << "\n" ); exit(1); }



using namespace std;

// Using declarations for common LLVM types
using llvm::raw_ostream;
using llvm::StringRef;
using llvm::StringMap;
using llvm::StringSet;
using llvm::SmallVector;
using llvm::SmallString;
using llvm::DenseMap;
using llvm::ArrayRef;
using llvm::Error;




// Type aliases for fixed-width integer types
using i8   = int8_t;
using u8   = uint8_t;
using i16  = int16_t;
using u16  = uint16_t;
using i32  = int32_t;
using u32  = uint32_t;
using i64  = int64_t;
using u64  = uint64_t;

// Type aliases for standard integer types
using isize = std::ptrdiff_t;
using usize = std::size_t;

// Type aliases for character types
using c8   = char;
using c16  = char16_t;
using c32  = char32_t;
using wchar = wchar_t;

// Type aliases for floating-point types
using f32  = float;
using f64  = double;
using f128 = long double;

// Type alias for boolean type
using b8   = bool;
using b32  = int32_t;


template<typename T>
using p_ = std::unique_ptr<T>;

template<typename T>
using s_ = std::shared_ptr<T>;

template<typename T, typename... Args>
p_<T> make_(Args&&... args) 
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}
template<typename T, typename... Args>
p_<T> make_shared(Args&&... args) 
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

#define new_(T, V, ...) if (auto V = make_<T>(__VA_ARGS__))
#define new_s(T, V, ...) if (auto V = make_shared<T>(__VA_ARGS__))

#define a_ auto