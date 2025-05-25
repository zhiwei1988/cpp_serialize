#include <gtest/gtest.h>
#include "string_literal.h"
#include <string>
#include <array>

// 测试 StringLiteral 的基本功能
TEST(StringLiteralTest, BasicConstruction) {
    // 测试从字符串字面量构造
    constexpr auto lit1 = csrl::make_string_literal("hello");
    EXPECT_EQ(lit1.str(), "hello");
}

// 测试空字符串
TEST(StringLiteralTest, EmptyString) {
    constexpr auto empty_lit = csrl::make_string_literal("");
    EXPECT_EQ(empty_lit.str(), "");
    EXPECT_EQ(empty_lit.str().length(), 0);
}

// 测试单字符字符串
TEST(StringLiteralTest, SingleCharacter) {
    constexpr auto single_lit = csrl::make_string_literal("a");
    EXPECT_EQ(single_lit.str(), "a");
    EXPECT_EQ(single_lit.str().length(), 1);
}

// 测试较长的字符串
TEST(StringLiteralTest, LongString) {
    constexpr auto long_lit = csrl::make_string_literal("This is a longer string for testing purposes");
    EXPECT_EQ(long_lit.str(), "This is a longer string for testing purposes");
    EXPECT_EQ(long_lit.str().length(), 44);
}

// 测试包含特殊字符的字符串
TEST(StringLiteralTest, SpecialCharacters) {
    constexpr auto special_lit = csrl::make_string_literal("Hello\nWorld\t!");
    EXPECT_EQ(special_lit.str(), "Hello\nWorld\t!");
    
    constexpr auto unicode_lit = csrl::make_string_literal("测试中文");
    EXPECT_EQ(unicode_lit.str(), "测试中文");
}

// 测试数组访问
TEST(StringLiteralTest, ArrayAccess) {
    constexpr auto lit = csrl::make_string_literal("test");
    
    // 测试数组内容
    EXPECT_EQ(lit.arr_[0], 't');
    EXPECT_EQ(lit.arr_[1], 'e');
    EXPECT_EQ(lit.arr_[2], 's');
    EXPECT_EQ(lit.arr_[3], 't');
    EXPECT_EQ(lit.arr_[4], '\0');  // 空终止符
}

// 测试编译时构造
TEST(StringLiteralTest, CompileTimeConstruction) {
    // 这些都应该在编译时完成
    constexpr auto lit1 = csrl::make_string_literal("compile_time");
    constexpr auto lit2 = csrl::make_string_literal("test");
    
    // 在编译时可以访问数组
    static_assert(lit1.arr_[0] == 'c', "First character should be 'c'");
    static_assert(lit2.arr_[0] == 't', "First character should be 't'");
    
    EXPECT_EQ(lit1.str(), "compile_time");
    EXPECT_EQ(lit2.str(), "test");
}

// 测试不同长度的字符串
TEST(StringLiteralTest, DifferentLengths) {
    constexpr auto short_lit = csrl::make_string_literal("hi");
    constexpr auto medium_lit = csrl::make_string_literal("hello world");
    constexpr auto long_lit = csrl::make_string_literal("this is a very long string for testing");
    
    EXPECT_EQ(short_lit.str(), "hi");
    EXPECT_EQ(medium_lit.str(), "hello world");
    EXPECT_EQ(long_lit.str(), "this is a very long string for testing");
    
    EXPECT_EQ(short_lit.str().length(), 2);
    EXPECT_EQ(medium_lit.str().length(), 11);
    EXPECT_EQ(long_lit.str().length(), 38);
}

// 测试模板参数推导
TEST(StringLiteralTest, TemplateDeduction) {
    // 测试不同长度的模板参数是否正确推导
    auto lit3 = csrl::make_string_literal("123");     // N = 4
    auto lit5 = csrl::make_string_literal("12345");   // N = 6
    auto lit10 = csrl::make_string_literal("1234567890"); // N = 11
    
    EXPECT_EQ(lit3.str(), "123");
    EXPECT_EQ(lit5.str(), "12345");
    EXPECT_EQ(lit10.str(), "1234567890");
}