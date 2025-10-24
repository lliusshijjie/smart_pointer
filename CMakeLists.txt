cmake_minimum_required(VERSION 3.12)
project(SmartPointer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译器优化设置
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
elseif(MSVC)
    add_definitions(-D_SILENCE_CXX17_C_HEADER_DEPRECATION_WARNING)
    add_definitions(-DNOMINMAX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
endif()

# 包含目录
include_directories(include)

# 主库（头文件库）
add_library(my_smart_ptr INTERFACE)
target_include_directories(my_smart_ptr INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 性能测试程序
add_executable(benchmark ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmark.cpp)
target_link_libraries(benchmark my_smart_ptr)

# 单元测试程序
add_executable(test_smart_ptr ${CMAKE_CURRENT_SOURCE_DIR}/src/test_smart_ptr.cpp)
target_link_libraries(test_smart_ptr my_smart_ptr)

# 安装头文件
install(DIRECTORY include/ DESTINATION include)