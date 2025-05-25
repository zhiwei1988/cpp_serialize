cmake_minimum_required(VERSION 3.14)
project(cpp_serialize VERSION 0.1.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 添加可执行文件
add_executable(${PROJECT_NAME} src/main.cpp)

# 添加测试
add_subdirectory(test)