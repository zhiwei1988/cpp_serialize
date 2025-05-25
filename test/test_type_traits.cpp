#include <gtest/gtest.h>
#include "define_type_traits.h"
#include <type_traits>
#include <string>

template<typename T, typename U>
constexpr bool is_same_v() {
    return std::is_same<T, U>::value;
}
// 测试 remove_cvref_t 类型特征
class TypeTraitsTest : public ::testing::Test {
};

// 测试基本类型
TEST_F(TypeTraitsTest, BasicTypes) {
    // 测试基本类型不变
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<int>, int>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<double>, double>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<char>, char>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<bool>, bool>));
}

// 测试移除const修饰符
TEST_F(TypeTraitsTest, RemoveConst) {
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const int>, int>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const double>, double>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const char>, char>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const std::string>, std::string>));
}

// 测试移除volatile修饰符
TEST_F(TypeTraitsTest, RemoveVolatile) {
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<volatile int>, int>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<volatile double>, double>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<volatile char>, char>));
}

// 测试移除const volatile修饰符
TEST_F(TypeTraitsTest, RemoveConstVolatile) {
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const volatile int>, int>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const volatile double>, double>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const volatile char>, char>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const volatile std::string>, std::string>));
}

// 测试移除左值引用
TEST_F(TypeTraitsTest, RemoveLValueReference) {
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<int&>, int>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<double&>, double>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<std::string&>, std::string>));
}

// 测试移除右值引用
TEST_F(TypeTraitsTest, RemoveRValueReference) {
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<int&&>, int>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<double&&>, double>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<std::string&&>, std::string>));
}

// 测试移除const引用
TEST_F(TypeTraitsTest, RemoveConstReference) {
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const int&>, int>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const double&>, double>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const std::string&>, std::string>));
    
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const int&&>, int>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const double&&>, double>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const std::string&&>, std::string>));
}

// 测试移除volatile引用
TEST_F(TypeTraitsTest, RemoveVolatileReference) {
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<volatile int&>, int>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<volatile double&>, double>));
    
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<volatile int&&>, int>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<volatile double&&>, double>));
}

// 测试移除const volatile引用（最复杂的情况）
TEST_F(TypeTraitsTest, RemoveConstVolatileReference) {
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const volatile int&>, int>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const volatile double&>, double>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const volatile std::string&>, std::string>));
    
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const volatile int&&>, int>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const volatile double&&>, double>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const volatile std::string&&>, std::string>));
}

// 测试指针类型（指针本身不应该被移除）
TEST_F(TypeTraitsTest, PointerTypes) {
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<int*>, int*>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const int*>, const int*>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<int* const>, int*>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const int* const>, const int*>));
    
    // 但是指针的引用和cv修饰符应该被移除
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<int*&>, int*>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<int* const&>, int*>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const int* const&>, const int*>));
}

// 测试数组类型
TEST_F(TypeTraitsTest, ArrayTypes) {
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<int[10]>, int[10]>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const int[10]>, int[10]>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<int(&)[10]>, int[10]>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const int(&)[10]>, int[10]>));
}

// 测试函数类型
TEST_F(TypeTraitsTest, FunctionTypes) {
    using FuncType = int(double);
    using FuncPtr = int(*)(double);
    
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<FuncType>, FuncType>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<FuncPtr>, FuncPtr>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<FuncType&>, FuncType>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<FuncPtr&>, FuncPtr>));
}

// 测试自定义类型
TEST_F(TypeTraitsTest, CustomTypes) {
    struct TestStruct {
        int value;
    };
    
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<TestStruct>, TestStruct>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const TestStruct>, TestStruct>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<volatile TestStruct>, TestStruct>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const volatile TestStruct>, TestStruct>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<TestStruct&>, TestStruct>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<TestStruct&&>, TestStruct>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const TestStruct&>, TestStruct>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const volatile TestStruct&&>, TestStruct>));
}

// 测试与标准库的兼容性（如果C++17可用）
#if __cplusplus >= 201703L
TEST_F(TypeTraitsTest, CompareWithStdRemoveCvref) {
    // 与标准库的 std::remove_cvref_t 进行比较
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const volatile int&>, std::remove_cvref_t<const volatile int&>>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<const std::string&&>, std::remove_cvref_t<const std::string&&>>));
    EXPECT_TRUE((is_same_v<csrl::remove_cvref_t<volatile double&>, std::remove_cvref_t<volatile double&>>));
}
#endif