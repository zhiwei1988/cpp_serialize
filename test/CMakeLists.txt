cmake_minimum_required(VERSION 3.10)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用测试(CTest 是 CMake 的测试框架)
# 启用后，可以使用 add_test() 命令添加测试用例
enable_testing()

# 包含 CMake 的 FetchContent 模块
# FetchContent 是 CMake 3.11 引入的一个模块，用于在配置时下载外部依赖
include(FetchContent)

# 添加Google Test
# 声明要下载的 Google Test 库
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git # 指定了 Google Test 的 Git 仓库地址
    GIT_TAG release-1.12.1 # 指定了要下载的版本标签
)

# 下载 Google Test 源代码，配置 Google Test 项目，并将其添加到当前项目中
FetchContent_MakeAvailable(googletest)

# 添加测试可执行文件
add_executable(test_cpp_serialize test_tuple_interface.cpp test_type_traits.cpp test_string_literal.cpp)

target_include_directories(test_cpp_serialize PRIVATE ${PROJECT_SOURCE_DIR}/include)

# 链接 GTest 库
target_link_libraries(test_cpp_serialize gtest gtest_main)

# 添加测试
add_test(NAME test_all COMMAND test_cpp_serialize)