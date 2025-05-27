#include <gtest/gtest.h>
#include "field_access.h"
#include "field_mapping.h"

using namespace csrl;

// 测试用的结构体
struct TestStruct {
    int x;
    double y;
    std::string z;

    MAKE_VISITABLE(x, y, z);
};

// 测试 IsVisitable 特性
TEST(FieldAccessTest, IsVisitable) {
    EXPECT_TRUE(csrl::IsVisitable<TestStruct>::value);
    EXPECT_FALSE(csrl::IsVisitable<int>::value);
}

// 测试 GetField 函数
TEST(FieldAccessTest, GetField) {
    TestStruct ts{42, 3.14, "hello"};
    
    // 测试非常量访问
    EXPECT_EQ(csrl::GetField<0>(ts), 42);
    EXPECT_DOUBLE_EQ(csrl::GetField<1>(ts), 3.14);
    EXPECT_EQ(csrl::GetField<2>(ts), "hello");

    // 测试常量访问
    const TestStruct cts{42, 3.14, "hello"};
    EXPECT_EQ(csrl::GetField<0>(cts), 42);
    EXPECT_DOUBLE_EQ(csrl::GetField<1>(cts), 3.14);
    EXPECT_EQ(csrl::GetField<2>(cts), "hello");
}

// 测试 FieldType 类型别名
TEST(FieldAccessTest, FieldType) {
    EXPECT_TRUE((std::is_same<csrl::FieldType<0, TestStruct>, int>::value));
    EXPECT_TRUE((std::is_same<csrl::FieldType<1, TestStruct>, double>::value));
    EXPECT_TRUE((std::is_same<csrl::FieldType<2, TestStruct>, std::string>::value));
}

// 测试字段映射
TEST(FieldMappingTest, DefaultFieldMapping) {
    // 创建默认映射
    auto mapping = csrl::DefaultFieldMapping<0, 1>();
    
    int src = 42;
    double dst = 0.0;
    mapping.Convert(src, dst);
    EXPECT_DOUBLE_EQ(dst, 42.0);
}

// 测试自定义字段映射
TEST(FieldMappingTest, CustomFieldMapping) {
    // 创建自定义转换器
    auto converter = [](const std::string& src, int& dst) {
        dst = std::stoi(src);
    };
    
    auto mapping = csrl::FieldMappingWith<0, 1>(converter);
    
    std::string src = "42";
    int dst = 0;
    mapping.Convert(src, dst);
    EXPECT_EQ(dst, 42);
}