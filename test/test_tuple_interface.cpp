#include <gtest/gtest.h>
#include "bind_to_tuple.h"
#include "define_tuple_interface.h"
#include <string>
#include <type_traits>

DEFINE_STRUCT_WITH_TUPLE_INTERFACE(Person, (std::string, name), (int, age), (double, height))

// 测试结构体定义和元组接口
TEST(TupleInterfaceTest, BasicStructDefinition)
{
    Person p{"张三", 25, 1.75};

    // 测试直接访问成员
    EXPECT_EQ(p.name, "张三");
    EXPECT_EQ(p.age, 25);
    EXPECT_DOUBLE_EQ(p.height, 1.75);

    // 测试通过元组接口访问
    EXPECT_EQ(std::get<0>(p), "张三");
    EXPECT_EQ(std::get<1>(p), 25);
    EXPECT_DOUBLE_EQ(std::get<2>(p), 1.75);

    // 测试元组大小
    EXPECT_EQ(std::tuple_size<Person>::value, 3);
}

#if 0

// 测试 bind_to_tuple 功能
TEST(BindToTupleTest, TupleView)
{
    Person p{"李四", 30, 1.80};

    // 使用 TupleView 获取成员指针的元组
    auto view = csrl::TupleViewHelper<3>::TupleView(p);

    // 验证指针指向正确的成员
    EXPECT_EQ(std::get<0>(view), &p.name);
    EXPECT_EQ(std::get<1>(view), &p.age);
    EXPECT_EQ(std::get<2>(view), &p.height);

    // 通过指针修改值
    *std::get<0>(view) = "王五";
    *std::get<1>(view) = 35;
    *std::get<2>(view) = 1.85;

    // 验证修改是否生效
    EXPECT_EQ(p.name, "王五");
    EXPECT_EQ(p.age, 35);
    EXPECT_DOUBLE_EQ(p.height, 1.85);
}

// 测试空结构体
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(EmptyStruct)
TEST(TupleInterfaceTest, EmptyStruct)
{
    EmptyStruct e;
    EXPECT_EQ(std::tuple_size<EmptyStruct>::value, 0);
    auto view = csrl::TupleViewHelper<0>::TupleView(e);
    EXPECT_EQ(std::tuple_size<decltype(view)>::value, 0);
}

// 测试常量对象
TEST(TupleInterfaceTest, ConstObject)
{
    const Person p{"赵六", 40, 1.90};

    // 测试常量访问
    EXPECT_EQ(std::get<0>(p), "赵六");
    EXPECT_EQ(std::get<1>(p), 40);
    EXPECT_DOUBLE_EQ(std::get<2>(p), 1.90);

    // 测试移动语义
    Person p2 = std::move(Person{"钱七", 45, 1.95});
    EXPECT_EQ(p2.name, "钱七");
    EXPECT_EQ(p2.age, 45);
    EXPECT_DOUBLE_EQ(p2.height, 1.95);
}
#endif
