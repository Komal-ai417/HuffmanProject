cmake_minimum_required(VERSION 3.10)
project(HuffmanProject)

# Require C++17 for unique_ptr and modern heap syntax
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add Release optimization flags by default
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# Platform specific optimizations
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /W4")
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/HuffmanCoder.cpp
)

# Main Executable creation
add_executable(huffman ${SOURCES})
target_include_directories(huffman PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- TESTING ---
enable_testing()

add_executable(huffman_test tests/test_huffman.cpp src/HuffmanCoder.cpp)
target_include_directories(huffman_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_test(NAME core_huffman_test COMMAND huffman_test)
