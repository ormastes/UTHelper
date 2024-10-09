# toolchain.cmake
# Set the compiler to clang
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 17)

# Set the flags for C and C++
set(CMAKE_C_FLAGS "-Wall -Wextra -O2")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2")

# Set the build type if not already set

# Include directories and link directories if needed
# (e.g., include path to standard C++ libraries)
