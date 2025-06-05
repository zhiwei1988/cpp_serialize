#include <gtest/gtest.h>
#include "define_tuple_interface.h"
#include "field_mapping.h"
#include "field_convert.h"
#include <string>
#include <cstring>

using namespace csrl;

// 定义测试用的结构体
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(SimpleSource, (int, id), (float, value))
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(SimpleTarget, (float, val), (int, identifier))

DEFINE_STRUCT_WITH_TUPLE_INTERFACE(InnerStruct, (int, a), (float, b), (double, c))
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(NestedSource, (InnerStruct, inner), (int, count))
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(NestedTarget, (InnerStruct, inner), (int, count))

using CharArray = char[32];
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(StringToCharTest, (std::string, name), (int, id))
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(CharToStringTest, (CharArray, name), (int, id))

// 测试 FieldPath 基本属性
TEST(FieldPathTest, BasicProperties)
{
    // 测试空路径
    using EmptyPath = decltype(csrl::MakeFieldPath<>());
    EXPECT_TRUE(EmptyPath::is_empty);
    EXPECT_EQ(EmptyPath::depth, 0);

    // 测试单层路径
    using SinglePath = decltype(csrl::MakeFieldPath<0>());
    EXPECT_FALSE(SinglePath::is_empty);
    EXPECT_EQ(SinglePath::depth, 1);

    // 测试多层路径
    using MultiPath = decltype(csrl::MakeFieldPath<0, 1, 2>());
    EXPECT_FALSE(MultiPath::is_empty);
    EXPECT_EQ(MultiPath::depth, 3);
}

// 测试 PathAccessor 字段访问
TEST(PathAccessorTest, FieldAccess)
{
    // 创建测试结构体
    NestedSource nested{{10, 2.5f, 3.14}, 100};

    // 测试单层访问
    auto& field0 = GetFieldByPath(nested, MakeFieldPath<0>());
    EXPECT_EQ(field0.a, 10);
    EXPECT_FLOAT_EQ(field0.b, 2.5f);
    EXPECT_DOUBLE_EQ(field0.c, 3.14);

    auto& field1 = GetFieldByPath(nested, MakeFieldPath<1>());
    EXPECT_EQ(field1, 100);

    // 测试嵌套访问
    auto& nestedField = GetFieldByPath(nested, MakeFieldPath<0, 0>());
    EXPECT_EQ(nestedField, 10);

    // 测试修改字段
    GetFieldByPath(nested, MakeFieldPath<1>()) = 200;
    EXPECT_EQ(nested.count, 200);
}

// 测试默认字段映射规则（基本类型转换）
TEST(FieldMappingRuleTest, DefaultMappingBasicTypes) {
    SimpleSource src{42, 3.14f};
    SimpleTarget dst{0.0f, 0};
    
    // 创建映射规则：int -> int, float -> float
    auto rule1 = MakeFieldMappingRule(MakeFieldPath<0>(), MakeFieldPath<1>());
    auto rule2 = MakeFieldMappingRule(MakeFieldPath<1>(), MakeFieldPath<0>());
    
    // 执行映射
    rule1.Convert(src, dst);
    rule2.Convert(src, dst);
    
    // 验证结果
    EXPECT_EQ(dst.identifier, 42);
    EXPECT_FLOAT_EQ(dst.val, 3.14f);
}

// 测试类型转换映射（int -> float）
TEST(FieldMappingRuleTest, TypeConversion) {
    SimpleSource src{100, 2.5f};
    SimpleTarget dst{0.0f, 0};
    
    // int -> float 转换
    auto rule = MakeFieldMappingRule(MakeFieldPath<0>(), MakeFieldPath<0>());
    rule.Convert(src, dst);
    
    EXPECT_FLOAT_EQ(dst.val, 100.0f);
}

// 测试自定义字段映射规则
TEST(FieldMappingCustomRuleTest, CustomConverter) {
    SimpleSource src{10, 5.0f};
    SimpleTarget dst{0.0f, 0};
    
    // 创建自定义转换器：将 int 乘以 2 后转为 float
    auto converter = [](const int& srcVal, float& dstVal) {
        dstVal = static_cast<float>(srcVal * 2);
    };
    
    auto rule = MakeFieldMappingCustomRule(MakeFieldPath<0>(), MakeFieldPath<0>(), converter);
    rule.Convert(src, dst);
    
    EXPECT_FLOAT_EQ(dst.val, 20.0f);
}

// 测试 StringToCharArrayConverter
TEST(ConverterTest, StringToCharArray) {
    std::string src = "hello world";
    char dst[32];
    
    // 清空目标数组
    memset(dst, 0, sizeof(dst));
    
    // 执行转换
    StringToCharArrayConverter(src, dst);
    
    // 验证结果
    EXPECT_STREQ(dst, "hello world");
}

// 测试 CharArrayToStringConverter
TEST(ConverterTest, CharArrayToString) {
    char src[32] = "test string";
    std::string dst;
    
    // 执行转换
    CharArrayToStringConverter(src, dst);
    
    // 验证结果
    EXPECT_EQ(dst, "test string");
}

// 测试 string 到 char 数组的字段映射
TEST(FieldMappingCustomRuleTest, StringToCharArrayMapping) {
    StringToCharTest src{"hello", 123};
    CharToStringTest dst{"", 0};
    
    // 创建带转换器的映射规则
    auto rule = MakeFieldMappingCustomRule(
        MakeFieldPath<0>(), 
        MakeFieldPath<0>(), 
        StringToCharArrayConverter<std::string, CharArray>
    );
    
    // 执行映射
    rule.Convert(src, dst);
    
    // 验证结果
    EXPECT_STREQ(dst.name, "hello");
}

// 测试 char 数组到 string 的字段映射
TEST(FieldMappingCustomRuleTest, CharArrayToStringMapping) {
    CharToStringTest src{"world", 456};
    StringToCharTest dst{"", 0};
    
    // 先设置源数据
    strcpy(src.name, "world");
    
    // 创建带转换器的映射规则
    auto rule = MakeFieldMappingCustomRule(
        MakeFieldPath<0>(), 
        MakeFieldPath<0>(), 
        CharArrayToStringConverter<CharArray, std::string>
    );
    
    // 执行映射
    rule.Convert(src, dst);
    
    // 验证结果
    EXPECT_EQ(dst.name, "world");
}

// 测试结构体字段映射规则
TEST(StructFieldMappingRuleTest, NestedStructMapping) {
    NestedSource src{{5, 1.5f, 2.5}, 50};
    NestedTarget dst{{0, 0.0f, 0.0}, 0};
    
    // 创建内部结构的映射规则
    auto innerRules = MakeMappingRuleTuple(
        MakeFieldMappingRule(MakeFieldPath<0>(), MakeFieldPath<0>()),  // int -> int
        MakeFieldMappingRule(MakeFieldPath<1>(), MakeFieldPath<1>()),  // float -> float
        MakeFieldMappingRule(MakeFieldPath<2>(), MakeFieldPath<2>())   // double -> double
    );
    
    // 创建结构体映射规则
    auto structRule = MakeStructFieldMappingRule(
        MakeFieldPath<0>(), 
        MakeFieldPath<0>(), 
        innerRules
    );
    
    // 执行映射
    structRule.Convert(src, dst);
    
    // 验证结果
    EXPECT_EQ(dst.inner.a, 5);
    EXPECT_FLOAT_EQ(dst.inner.b, 1.5f);
    EXPECT_DOUBLE_EQ(dst.inner.c, 2.5);
}

#if 0
// 测试 MappingRuleTuple 创建和访问
TEST(MappingRuleTupleTest, Creation) {
    // 创建多个映射规则
    auto rule1 = MakeFieldMappingRule(MakeFieldPath<0>(), MakeFieldPath<0>());
    auto rule2 = MakeFieldMappingRule(MakeFieldPath<1>(), MakeFieldPath<1>());
    
    // 创建映射规则集合
    auto ruleTuple = MakeMappingRuleTuple(std::move(rule1), std::move(rule2));
    
    // 验证属性
    EXPECT_EQ(ruleTuple.size, 2);
    
    // 测试访问（编译时测试）
    auto& firstMapping = ruleTuple.GetMapping<0>();
    auto& secondMapping = ruleTuple.GetMapping<1>();
    
    // 只要能编译通过就说明访问正常
    EXPECT_TRUE(true);
}
#endif

// 测试完整的结构体转换流程
TEST(StructFieldsConvertTest, CompleteConversion) {
    SimpleSource src{42, 3.14f};
    SimpleTarget dst{0.0f, 0};
    
    // 创建映射规则集合
    auto mappingTuple = MakeMappingRuleTuple(
        MakeFieldMappingRule(MakeFieldPath<1>(), MakeFieldPath<0>()),  // float -> float
        MakeFieldMappingRule(MakeFieldPath<0>(), MakeFieldPath<1>())   // int -> int
    );
    
    // 执行完整转换
    StructFieldsConvert(src, dst, mappingTuple);
    
    // 验证结果
    EXPECT_FLOAT_EQ(dst.val, 3.14f);
    EXPECT_EQ(dst.identifier, 42);
}

// 测试带自定义转换器的完整转换流程
TEST(StructFieldsConvertTest, CustomConversionComplete) {
    SimpleSource src{10, 2.5f};
    SimpleTarget dst{0.0f, 0};
    
    // 创建带自定义转换器的映射规则集合
    auto mappingTuple = MakeMappingRuleTuple(
        MakeFieldMappingCustomRule(MakeFieldPath<1>(), MakeFieldPath<0>(), 
            [](const float& srcVal, float& dstVal) {
                dstVal = srcVal * 2.0f;
            }),
        MakeFieldMappingCustomRule(MakeFieldPath<0>(), MakeFieldPath<1>(), 
            [](const int& srcVal, int& dstVal) {
                dstVal = srcVal + 100;
            })
    );
    
    // 执行完整转换
    StructFieldsConvert(src, dst, mappingTuple);
    
    // 验证结果
    EXPECT_FLOAT_EQ(dst.val, 5.0f);  // 2.5 * 2
    EXPECT_EQ(dst.identifier, 110);   // 10 + 100
}

// 测试嵌套结构体的完整转换流程
TEST(StructFieldsConvertTest, NestedStructConversion) {
    NestedSource src{{5, 1.5f, 2.5}, 50};
    NestedTarget dst{{0, 0.0f, 0.0}, 0};
    
    // 创建嵌套映射规则
    auto mappingTuple = MakeMappingRuleTuple(
        MakeStructFieldMappingRule(MakeFieldPath<0>(), MakeFieldPath<0>(), 
            MakeMappingRuleTuple(
                MakeFieldMappingRule(MakeFieldPath<0>(), MakeFieldPath<0>()),
                MakeFieldMappingRule(MakeFieldPath<1>(), MakeFieldPath<1>()),
                MakeFieldMappingRule(MakeFieldPath<2>(), MakeFieldPath<2>())
            )),
        MakeFieldMappingRule(MakeFieldPath<1>(), MakeFieldPath<1>())
    );
    
    // 执行完整转换
    StructFieldsConvert(src, dst, mappingTuple);
    
    // 验证结果
    EXPECT_EQ(dst.inner.a, 5);
    EXPECT_FLOAT_EQ(dst.inner.b, 1.5f);
    EXPECT_DOUBLE_EQ(dst.inner.c, 2.5);
    EXPECT_EQ(dst.count, 50);
}