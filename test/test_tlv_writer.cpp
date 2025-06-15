/**
 * @file test_tlv_writer.cpp
 * @author Zhiwei Tan (zhiweix1988@gmail.com)
 * @brief TLV 写入器测试
 * @version 0.1
 * @date 2025-06-14 10:00:00
 * 
 * @copyright Copyright (c) 2025
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "define_tuple_interface.h"
#include "field_convert.h"
#include "tlv_writer.h"

using namespace csrl;

class TLVWriterTest : public ::testing::Test {
protected:
    void SetUp() override {
        writer = std::make_unique<TLVWriter>(1024);
    }
    
    void TearDown() override {
        writer.reset();
    }
    
    std::unique_ptr<TLVWriter> writer;
};

// 测试基本的 AppendBuf 功能
TEST_F(TLVWriterTest, AppendBuf_Basic) {
    const char* testData = "Hello";
    uint32_t type = 0x1001;
    size_t dataLen = strlen(testData);
    
    int32_t result = writer->AppendBuf(type, testData, dataLen);
    EXPECT_EQ(result, 0);
    
    // 验证写入的数据格式：type(4) + length(4) + value(5) = 13字节
    const uint8_t* data = writer->data();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(writer->size(), 2 * sizeof(uint32_t) + dataLen);
    
    // 验证 type
    uint32_t actualType;
    memcpy(&actualType, data, sizeof(uint32_t));
    EXPECT_EQ(actualType, type);
    
    // 验证 length
    uint32_t actualLength;
    memcpy(&actualLength, data + sizeof(uint32_t), sizeof(uint32_t));
    EXPECT_EQ(actualLength, dataLen);
    
    // 验证 value
    EXPECT_EQ(memcmp(data + 2 * sizeof(uint32_t), testData, dataLen), 0);
}

// 测试空数据的处理
TEST_F(TLVWriterTest, AppendBuf_EmptyData) {
    uint32_t type = 0x1002;
    
    int32_t result = writer->AppendBuf(type, nullptr, 0);
    EXPECT_EQ(result, 0);
    
    // 验证写入的数据格式：type(4) + length(4) + value(0) = 8字节
    const uint8_t* data = writer->data();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(writer->size(), 2 * sizeof(uint32_t));
    
    // 验证 type
    uint32_t actualType;
    memcpy(&actualType, data, sizeof(uint32_t));
    EXPECT_EQ(actualType, type);
    
    // 验证 length
    uint32_t actualLength;
    memcpy(&actualLength, data + sizeof(uint32_t), sizeof(uint32_t));
    EXPECT_EQ(actualLength, 0);
}

// 测试多次调用 AppendBuf
TEST_F(TLVWriterTest, AppendBuf_Multiple) {
    const char* firstData = "First";
    const char* secondData = "Second";
    uint32_t firstType = 0x1003;
    uint32_t secondType = 0x1004;
    size_t firstLen = strlen(firstData);
    size_t secondLen = strlen(secondData);
    
    // 第一次写入
    int32_t result1 = writer->AppendBuf(firstType, firstData, firstLen);
    EXPECT_EQ(result1, 0);
    
    size_t sizeAfterFirst = writer->size();
    EXPECT_EQ(sizeAfterFirst, 2 * sizeof(uint32_t) + firstLen);
    
    // 第二次写入
    int32_t result2 = writer->AppendBuf(secondType, secondData, secondLen);
    EXPECT_EQ(result2, 0);
    
    size_t sizeAfterSecond = writer->size();
    EXPECT_EQ(sizeAfterSecond, 4 * sizeof(uint32_t) + firstLen + secondLen);
    
    const uint8_t* data = writer->data();
    ASSERT_NE(data, nullptr);
    
    // 验证第一个 TLV 记录
    size_t offset = 0;
    
    uint32_t actualType1;
    memcpy(&actualType1, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualType1, firstType);
    offset += sizeof(uint32_t);
    
    uint32_t actualLength1;
    memcpy(&actualLength1, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualLength1, firstLen);
    offset += sizeof(uint32_t);
    
    EXPECT_EQ(memcmp(data + offset, firstData, firstLen), 0);
    offset += firstLen;
    
    // 验证第二个 TLV 记录
    uint32_t actualType2;
    memcpy(&actualType2, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualType2, secondType);
    offset += sizeof(uint32_t);
    
    uint32_t actualLength2;
    memcpy(&actualLength2, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualLength2, secondLen);
    offset += sizeof(uint32_t);
    
    EXPECT_EQ(memcmp(data + offset, secondData, secondLen), 0);
}

// 测试大数据处理（触发内部 resize）
TEST_F(TLVWriterTest, AppendBuf_LargeData) {
    // 创建一个较小的 writer 来测试 resize 功能
    TLVWriter smallWriter(16);
    
    // 创建一个大于初始容量的数据
    const size_t largeDataSize = 10000;
    char* largeData = new char[largeDataSize];
    memset(largeData, 'A', largeDataSize);
    uint32_t type = 0x1005;
    
    int32_t result = smallWriter.AppendBuf(type, largeData, largeDataSize);
    EXPECT_EQ(result, 0);
    
    // 验证数据正确性
    const uint8_t* data = smallWriter.data();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(smallWriter.size(), 2 * sizeof(uint32_t) + largeDataSize);
    
    uint32_t actualType;
    memcpy(&actualType, data, sizeof(uint32_t));
    EXPECT_EQ(actualType, type);
    
    uint32_t actualLength;
    memcpy(&actualLength, data + sizeof(uint32_t), sizeof(uint32_t));
    EXPECT_EQ(actualLength, largeDataSize);
    
    EXPECT_EQ(memcmp(data + 2 * sizeof(uint32_t), largeData, largeDataSize), 0);
    
    delete[] largeData;
}

// 测试不同类型的数据
TEST_F(TLVWriterTest, AppendBuf_DifferentTypes) {
    struct TestCase {
        uint32_t type;
        const char* data;
        size_t length;
    };
    
    TestCase testCases[] = {
        {0x2001, "Short", 5},
        {0x2002, "Medium length string", 20},
        {0x2003, "", 0},
        {0x2004, "Special chars: !@#$%^&*()", 24}
    };
    
    size_t expectedSize = 0;
    for (const auto& testCase : testCases) {
        int32_t result = writer->AppendBuf(testCase.type, testCase.data, testCase.length);
        EXPECT_EQ(result, 0);
        expectedSize += 2 * sizeof(uint32_t) + testCase.length;
    }
    
    // 验证总大小
    EXPECT_EQ(writer->size(), expectedSize);
    
    // 验证所有数据都正确写入
    const uint8_t* data = writer->data();
    ASSERT_NE(data, nullptr);
    
    size_t offset = 0;
    for (const auto& testCase : testCases) {
        uint32_t actualType;
        memcpy(&actualType, data + offset, sizeof(uint32_t));
        EXPECT_EQ(actualType, testCase.type);
        offset += sizeof(uint32_t);
        
        uint32_t actualLength;
        memcpy(&actualLength, data + offset, sizeof(uint32_t));
        EXPECT_EQ(actualLength, testCase.length);
        offset += sizeof(uint32_t);
        
        if (testCase.length > 0) {
            EXPECT_EQ(memcmp(data + offset, testCase.data, testCase.length), 0);
        }
        offset += testCase.length;
    }
}

// 测试 AppendPair 接口
TEST_F(TLVWriterTest, AppendPair_Basic) {
    const char* firstBuf = "Hello";
    const char* secondBuf = "World";
    uint32_t type = 0x3001;
    size_t firstLen = strlen(firstBuf);
    size_t secondLen = strlen(secondBuf);
    
    int32_t result = writer->AppendPair(type, firstBuf, firstLen, secondBuf, secondLen);
    EXPECT_EQ(result, 0);
    
    // 验证写入的数据格式：type(4) + length(4) + value(10) = 18字节
    const uint8_t* data = writer->data();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(writer->size(), 2 * sizeof(uint32_t) + firstLen + secondLen);
    
    // 验证 type
    uint32_t actualType;
    memcpy(&actualType, data, sizeof(uint32_t));
    EXPECT_EQ(actualType, type);
    
    // 验证 length（应该是两段数据的总长度）
    uint32_t actualLength;
    memcpy(&actualLength, data + sizeof(uint32_t), sizeof(uint32_t));
    EXPECT_EQ(actualLength, firstLen + secondLen);
    
    // 验证拼接的 value
    size_t offset = 2 * sizeof(uint32_t);
    EXPECT_EQ(memcmp(data + offset, firstBuf, firstLen), 0);
    offset += firstLen;
    EXPECT_EQ(memcmp(data + offset, secondBuf, secondLen), 0);
}

// 测试 AppendPair 接口处理空数据
TEST_F(TLVWriterTest, AppendPair_EmptySegments) {
    const char* nonEmptyBuf = "Data";
    uint32_t type = 0x3002;
    size_t nonEmptyLen = strlen(nonEmptyBuf);
    
    // 第一段为空，第二段非空
    int32_t result1 = writer->AppendPair(type, nullptr, 0, nonEmptyBuf, nonEmptyLen);
    EXPECT_EQ(result1, 0);
    
    // 验证数据正确性
    const uint8_t* data = writer->data();
    size_t offset = 2 * sizeof(uint32_t);
    EXPECT_EQ(memcmp(data + offset, nonEmptyBuf, nonEmptyLen), 0);
    
    writer->clear();
    
    // 第一段非空，第二段为空
    int32_t result2 = writer->AppendPair(type, nonEmptyBuf, nonEmptyLen, nullptr, 0);
    EXPECT_EQ(result2, 0);
    
    // 验证数据正确性
    data = writer->data();
    offset = 2 * sizeof(uint32_t);
    EXPECT_EQ(memcmp(data + offset, nonEmptyBuf, nonEmptyLen), 0);
    
    writer->clear();
    
    // 两段都为空
    int32_t result3 = writer->AppendPair(type, nullptr, 0, nullptr, 0);
    EXPECT_EQ(result3, 0);
    EXPECT_EQ(writer->size(), 2 * sizeof(uint32_t));
}

// 测试 clear 方法
TEST_F(TLVWriterTest, Clear_Functionality) {
    const char* testData = "Test data";
    uint32_t type = 0x4001;
    size_t dataLen = strlen(testData);
    
    // 写入一些数据
    writer->AppendBuf(type, testData, dataLen);
    EXPECT_GT(writer->size(), 0);
    EXPECT_NE(writer->data(), nullptr);
    
    // 清空数据
    writer->clear();
    EXPECT_EQ(writer->size(), 0);
    
    // 清空后应该能继续写入数据
    writer->AppendBuf(type, testData, dataLen);
    EXPECT_EQ(writer->size(), 2 * sizeof(uint32_t) + dataLen);
}

// 测试 size 方法
TEST_F(TLVWriterTest, Size_Tracking) {
    EXPECT_EQ(writer->size(), 0);
    
    const char* testData1 = "First";
    const char* testData2 = "Second";
    uint32_t type1 = 0x5001;
    uint32_t type2 = 0x5002;
    size_t len1 = strlen(testData1);
    size_t len2 = strlen(testData2);
    
    // 第一次写入
    writer->AppendBuf(type1, testData1, len1);
    size_t expectedSize1 = 2 * sizeof(uint32_t) + len1;
    EXPECT_EQ(writer->size(), expectedSize1);
    
    // 第二次写入
    writer->AppendBuf(type2, testData2, len2);
    size_t expectedSize2 = expectedSize1 + 2 * sizeof(uint32_t) + len2;
    EXPECT_EQ(writer->size(), expectedSize2);
    
    // 使用 AppendPair
    writer->AppendPair(type1, testData1, len1, testData2, len2);
    size_t expectedSize3 = expectedSize2 + 2 * sizeof(uint32_t) + len1 + len2;
    EXPECT_EQ(writer->size(), expectedSize3);
}

using CharArray16 = char[16];
using IntArray4 = int32_t[4];

DEFINE_STRUCT_WITH_TUPLE_INTERFACE(TestArithmeticTLVConverter,
    (int32_t, intValue),
    (double, doubleValue),
    (uint32_t, uint32Value),
    (CharArray16, stringValue),
    (IntArray4, intArrayValue)
);

TEST_F(TLVWriterTest, BaseTLVConverter_Basic) {
    constexpr uint32_t INT_TYPE = 0x6001;
    constexpr uint32_t DOUBLE_TYPE = 0x6002;
    constexpr uint32_t UINT32_TYPE = 0x6003;
    constexpr uint32_t STRING_TYPE = 0x6004;
    constexpr uint32_t INT_ARRAY_TYPE = 0x6005;
    
    auto sharedWriter = std::shared_ptr<TLVWriter>(std::move(writer));

    auto mappingTuple = MakeMappingRuleTuple(
        MAKE_TLV_DEFAULT_MAPPING(MakeFieldPath<0>(), INT_TYPE),
        MAKE_TLV_DEFAULT_MAPPING(MakeFieldPath<1>(), DOUBLE_TYPE),
        MAKE_TLV_DEFAULT_MAPPING(MakeFieldPath<2>(), UINT32_TYPE),
        MAKE_TLV_DEFAULT_MAPPING(MakeFieldPath<3>(), STRING_TYPE),
        MAKE_TLV_DEFAULT_MAPPING(MakeFieldPath<4>(), INT_ARRAY_TYPE)
    );

    TestArithmeticTLVConverter testArithmeticTLVConverter{12345, 3.14159, 12345, "Hello", {100, 200, 300, 400}};
    StructFieldsConvert(testArithmeticTLVConverter, sharedWriter, mappingTuple);
    
    // 验证数据
    const uint8_t* data = sharedWriter->data();
    ASSERT_NE(data, nullptr);
    
    const char* expectedStringValue = "Hello";
    size_t expectedStringLen = strlen(expectedStringValue) + 1; // 包含null终止符
    size_t expectedIntArraySize = sizeof(testArithmeticTLVConverter.intArrayValue);
    size_t expectedSize = 5 * (2 * sizeof(uint32_t)) + sizeof(int32_t) + sizeof(double) + sizeof(uint32_t) + expectedStringLen + expectedIntArraySize;
    EXPECT_EQ(sharedWriter->size(), expectedSize);
    
    // 验证第一个 TLV（int）
    size_t offset = 0;
    uint32_t actualType1;
    memcpy(&actualType1, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualType1, INT_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t actualLength1;
    memcpy(&actualLength1, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualLength1, sizeof(int32_t));
    offset += sizeof(uint32_t);
    
    int32_t actualIntValue;
    memcpy(&actualIntValue, data + offset, sizeof(int32_t));
    EXPECT_EQ(actualIntValue, testArithmeticTLVConverter.intValue);
    offset += sizeof(int32_t);
    
    // 验证第二个 TLV（double）
    uint32_t actualType2;
    memcpy(&actualType2, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualType2, DOUBLE_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t actualLength2;
    memcpy(&actualLength2, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualLength2, sizeof(double));
    offset += sizeof(uint32_t);
    
    double actualDoubleValue;
    memcpy(&actualDoubleValue, data + offset, sizeof(double));
    EXPECT_DOUBLE_EQ(actualDoubleValue, testArithmeticTLVConverter.doubleValue);
    offset += sizeof(double);

    // 验证第三个 TLV（uint32）
    uint32_t actualType3;
    memcpy(&actualType3, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualType3, UINT32_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t actualLength3;
    memcpy(&actualLength3, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualLength3, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    uint32_t actualUint32Value;
    memcpy(&actualUint32Value, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualUint32Value, testArithmeticTLVConverter.uint32Value); 
    offset += sizeof(uint32_t);
    
    // 验证第四个 TLV（字符数组）
    uint32_t actualType4;
    memcpy(&actualType4, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualType4, STRING_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t actualLength4;
    memcpy(&actualLength4, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualLength4, expectedStringLen);
    offset += sizeof(uint32_t);
    
    std::string actualStringValue(reinterpret_cast<const char*>(data + offset), actualLength4 - 1); // 去掉null终止符进行比较
    EXPECT_EQ(actualStringValue, expectedStringValue);
    
    // 验证null终止符存在
    EXPECT_EQ(data[offset + actualLength4 - 1], '\0');
    offset += actualLength4;

    // 验证第五个 TLV（int32_t 数组）
    uint32_t actualType5;
    memcpy(&actualType5, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualType5, INT_ARRAY_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t actualLength5;
    memcpy(&actualLength5, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualLength5, expectedIntArraySize);
    offset += sizeof(uint32_t);
    
    // 验证int数组内容
    int32_t actualIntArray[4];
    memcpy(actualIntArray, data + offset, expectedIntArraySize);
    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(actualIntArray[i], testArithmeticTLVConverter.intArrayValue[i]);
    }
    offset += expectedIntArraySize;
}

DEFINE_STRUCT_WITH_TUPLE_INTERFACE(TestDigitalStringTLVConverter,
    (int32_t, intValue),
    (uint32_t, uint32Value),
    (int64_t, int64Value)
);

// 测试数字转字符串 TLV 转换器
TEST_F(TLVWriterTest, DigitalStringTLVConverter_Basic) {
    constexpr uint32_t INT_TYPE = 0x7001;
    constexpr uint32_t UINT32_TYPE = 0x7002;
    constexpr uint32_t INT64_TYPE = 0x7003;
    
    auto sharedWriter = std::shared_ptr<TLVWriter>(std::move(writer));

    auto mappingTuple = MakeMappingRuleTuple(
        MAKE_TLV_DIGITAL_STRING_MAPPING(MakeFieldPath<0>(), INT_TYPE),
        MAKE_TLV_DIGITAL_STRING_MAPPING(MakeFieldPath<1>(), UINT32_TYPE),
        MAKE_TLV_DIGITAL_STRING_MAPPING(MakeFieldPath<2>(), INT64_TYPE)
    );

    TestDigitalStringTLVConverter testDigitalStringTLVConverter{12345, 98765, 123456789LL};
    StructFieldsConvert(testDigitalStringTLVConverter, sharedWriter, mappingTuple);
    
    // 验证数据
    const uint8_t* data = sharedWriter->data();
    ASSERT_NE(data, nullptr);
    
    // 准备期望的字符串值
    std::string expectedIntStr = std::to_string(testDigitalStringTLVConverter.intValue);
    std::string expectedUint32Str = std::to_string(testDigitalStringTLVConverter.uint32Value);
    std::string expectedInt64Str = std::to_string(testDigitalStringTLVConverter.int64Value);
    
    size_t expectedSize = 3 * (2 * sizeof(uint32_t)) + expectedIntStr.length() + expectedUint32Str.length() + expectedInt64Str.length();
    EXPECT_EQ(sharedWriter->size(), expectedSize);
    
    // 验证第一个 TLV（int 转字符串）
    size_t offset = 0;
    uint32_t actualType1;
    memcpy(&actualType1, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualType1, INT_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t actualLength1;
    memcpy(&actualLength1, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualLength1, expectedIntStr.length());
    offset += sizeof(uint32_t);
    
    std::string actualIntStr(reinterpret_cast<const char*>(data + offset), actualLength1);
    EXPECT_EQ(actualIntStr, expectedIntStr);
    offset += actualLength1;
    
    // 验证第二个 TLV（uint32_t 转字符串）
    uint32_t actualType2;
    memcpy(&actualType2, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualType2, UINT32_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t actualLength2;
    memcpy(&actualLength2, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualLength2, expectedUint32Str.length());
    offset += sizeof(uint32_t);
    
    std::string actualUint32Str(reinterpret_cast<const char*>(data + offset), actualLength2);
    EXPECT_EQ(actualUint32Str, expectedUint32Str);
    offset += actualLength2;

    // 验证第三个 TLV（int64 转字符串）
    uint32_t actualType3;
    memcpy(&actualType3, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualType3, INT64_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t actualLength3;
    memcpy(&actualLength3, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualLength3, expectedInt64Str.length());
    offset += sizeof(uint32_t);

    std::string actualInt64Str(reinterpret_cast<const char*>(data + offset), actualLength3);
    EXPECT_EQ(actualInt64Str, expectedInt64Str);
    offset += actualLength3;
}

// 用于 MAKE_TLV_DEFAULT_MAPPING_WITH_KEY 测试的键名常量
static constexpr char INT_KEY_NAME[] = "intValue";
static constexpr char DOUBLE_KEY_NAME[] = "doubleValue";
static constexpr char UINT32_KEY_NAME[] = "uint32Value";

DEFINE_STRUCT_WITH_TUPLE_INTERFACE(TestTLVConverterWithKey,
    (int32_t, intValue),
    (double, doubleValue),
    (uint32_t, uint32Value)
);

// 测试带键名的 TLV 转换器
TEST_F(TLVWriterTest, BaseTLVConverterWithKey_Basic) {
    constexpr uint32_t INT_TYPE = 0x8001;
    constexpr uint32_t DOUBLE_TYPE = 0x8002;
    constexpr uint32_t UINT32_TYPE = 0x8003;
    
    auto sharedWriter = std::shared_ptr<TLVWriter>(std::move(writer));

    auto mappingTuple = MakeMappingRuleTuple(
        MAKE_TLV_DEFAULT_MAPPING_WITH_KEY(MakeFieldPath<0>(), INT_TYPE, INT_KEY_NAME),
        MAKE_TLV_DEFAULT_MAPPING_WITH_KEY(MakeFieldPath<1>(), DOUBLE_TYPE, DOUBLE_KEY_NAME),
        MAKE_TLV_DEFAULT_MAPPING_WITH_KEY(MakeFieldPath<2>(), UINT32_TYPE, UINT32_KEY_NAME)
    );

    TestTLVConverterWithKey testTLVConverterWithKey{54321, 2.71828, 98765};
    StructFieldsConvert(testTLVConverterWithKey, sharedWriter, mappingTuple);
    
    // 验证数据
    const uint8_t* data = sharedWriter->data();
    ASSERT_NE(data, nullptr);
    
    // 计算期望的总大小：每个TLV都是 type + length + (key + null + value)
    size_t intKeyLen = strlen(INT_KEY_NAME) + 1;
    size_t doubleKeyLen = strlen(DOUBLE_KEY_NAME) + 1;
    size_t uint32KeyLen = strlen(UINT32_KEY_NAME) + 1;
    
    size_t expectedSize = 3 * (2 * sizeof(uint32_t)) + 
                         (intKeyLen + sizeof(int32_t)) +
                         (doubleKeyLen + sizeof(double)) +
                         (uint32KeyLen + sizeof(uint32_t));
    EXPECT_EQ(sharedWriter->size(), expectedSize);
    
    // 验证第一个 TLV（int 带键名）
    size_t offset = 0;
    uint32_t actualType1;
    memcpy(&actualType1, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualType1, INT_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t actualLength1;
    memcpy(&actualLength1, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualLength1, intKeyLen + sizeof(int32_t));
    offset += sizeof(uint32_t);
    
    // 验证键名部分（包含null终止符）
    EXPECT_EQ(memcmp(data + offset, INT_KEY_NAME, intKeyLen), 0);
    offset += intKeyLen;
    
    // 验证值部分
    int32_t actualIntValue;
    memcpy(&actualIntValue, data + offset, sizeof(int32_t));
    EXPECT_EQ(actualIntValue, testTLVConverterWithKey.intValue);
    offset += sizeof(int32_t);
    
    // 验证第二个 TLV（double 带键名）
    uint32_t actualType2;
    memcpy(&actualType2, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualType2, DOUBLE_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t actualLength2;
    memcpy(&actualLength2, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualLength2, doubleKeyLen + sizeof(double));
    offset += sizeof(uint32_t);
    
    // 验证键名部分（包含null终止符）
    EXPECT_EQ(memcmp(data + offset, DOUBLE_KEY_NAME, doubleKeyLen), 0);
    offset += doubleKeyLen;
    
    // 验证值部分
    double actualDoubleValue;
    memcpy(&actualDoubleValue, data + offset, sizeof(double));
    EXPECT_DOUBLE_EQ(actualDoubleValue, testTLVConverterWithKey.doubleValue);
    offset += sizeof(double);

    // 验证第三个 TLV（uint32 带键名）
    uint32_t actualType3;
    memcpy(&actualType3, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualType3, UINT32_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t actualLength3;
    memcpy(&actualLength3, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualLength3, uint32KeyLen + sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // 验证键名部分（包含null终止符）
    EXPECT_EQ(memcmp(data + offset, UINT32_KEY_NAME, uint32KeyLen), 0);
    offset += uint32KeyLen;
    
    // 验证值部分
    uint32_t actualUint32Value;
    memcpy(&actualUint32Value, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualUint32Value, testTLVConverterWithKey.uint32Value);
    offset += sizeof(uint32_t);
}

// 定义子结构体（包含2个字段）
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(SubStruct,
    (int32_t, intField),
    (double, doubleField)
);

// 定义父结构体（包含一个子结构体）
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(ParentStruct,
    (SubStruct, subData)
);

// 用于带键值的子结构体测试
static constexpr char SUB_STRUCT_KEY_NAME[] = "subData";

// 测试子结构体 TLV 转换器（不带键值）
TEST_F(TLVWriterTest, SubStructTLVConverter_Basic) {
    constexpr uint32_t SUB_STRUCT_TYPE = 0x9001;
    constexpr uint32_t SUB_INT_TYPE = 0x9002;
    constexpr uint32_t SUB_DOUBLE_TYPE = 0x9003;
    
    auto sharedWriter = std::shared_ptr<TLVWriter>(std::move(writer));

    // 定义子结构体内部字段的转换规则
    auto subStructMappingTuple = MakeMappingRuleTuple(
        MAKE_TLV_DEFAULT_MAPPING(MakeFieldPath<0>(), SUB_INT_TYPE),
        MAKE_TLV_DEFAULT_MAPPING(MakeFieldPath<1>(), SUB_DOUBLE_TYPE)
    );

    // 定义父结构体的转换规则
    auto parentMappingTuple = MakeMappingRuleTuple(
        MAKE_TLV_SUB_STRUCT_MAPPING(MakeFieldPath<0>(), SUB_STRUCT_TYPE, subStructMappingTuple)
    );

    ParentStruct parentStruct{{123, 4.56}};
    StructFieldsConvert(parentStruct, sharedWriter, parentMappingTuple);
    
    // 验证数据
    const uint8_t* data = sharedWriter->data();
    ASSERT_NE(data, nullptr);
    
    // 预期结构：外层TLV包含内部的两个TLV
    // 内部数据大小：(int TLV: 4+4+4) + (double TLV: 4+4+8) = 28 字节
    // 外层数据大小：4+4+28 = 36 字节
    size_t expectedInnerSize = 2 * (2 * sizeof(uint32_t)) + sizeof(int32_t) + sizeof(double);
    size_t expectedOuterSize = 2 * sizeof(uint32_t) + expectedInnerSize;
    EXPECT_EQ(sharedWriter->size(), expectedOuterSize);
    
    // 验证外层 TLV
    size_t offset = 0;
    uint32_t actualType;
    memcpy(&actualType, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualType, SUB_STRUCT_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t actualLength;
    memcpy(&actualLength, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualLength, expectedInnerSize);
    offset += sizeof(uint32_t);
    
    // 验证内层第一个 TLV（int）
    uint32_t innerType1;
    memcpy(&innerType1, data + offset, sizeof(uint32_t));
    EXPECT_EQ(innerType1, SUB_INT_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t innerLength1;
    memcpy(&innerLength1, data + offset, sizeof(uint32_t));
    EXPECT_EQ(innerLength1, sizeof(int32_t));
    offset += sizeof(uint32_t);
    
    int32_t actualIntValue;
    memcpy(&actualIntValue, data + offset, sizeof(int32_t));
    EXPECT_EQ(actualIntValue, parentStruct.subData.intField);
    offset += sizeof(int32_t);
    
    // 验证内层第二个 TLV（double）
    uint32_t innerType2;
    memcpy(&innerType2, data + offset, sizeof(uint32_t));
    EXPECT_EQ(innerType2, SUB_DOUBLE_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t innerLength2;
    memcpy(&innerLength2, data + offset, sizeof(uint32_t));
    EXPECT_EQ(innerLength2, sizeof(double));
    offset += sizeof(uint32_t);
    
    double actualDoubleValue;
    memcpy(&actualDoubleValue, data + offset, sizeof(double));
    EXPECT_DOUBLE_EQ(actualDoubleValue, parentStruct.subData.doubleField);
}

// 测试子结构体 TLV 转换器（带键值）
TEST_F(TLVWriterTest, SubStructTLVConverterWithKey_Basic) {
    constexpr uint32_t SUB_STRUCT_TYPE = 0xA001;
    constexpr uint32_t SUB_INT_TYPE = 0xA002;
    constexpr uint32_t SUB_DOUBLE_TYPE = 0xA003;
    
    auto sharedWriter = std::shared_ptr<TLVWriter>(std::move(writer));

    // 定义子结构体内部字段的转换规则
    auto subStructMappingTuple = MakeMappingRuleTuple(
        MAKE_TLV_DEFAULT_MAPPING(MakeFieldPath<0>(), SUB_INT_TYPE),
        MAKE_TLV_DEFAULT_MAPPING(MakeFieldPath<1>(), SUB_DOUBLE_TYPE)
    );

    // 定义父结构体的转换规则（带键值）
    auto parentMappingTuple = MakeMappingRuleTuple(
        MAKE_TLV_SUB_STRUCT_MAPPING_WITH_KEY(MakeFieldPath<0>(), SUB_STRUCT_TYPE, subStructMappingTuple, SUB_STRUCT_KEY_NAME)
    );

    ParentStruct parentStruct{{789, 1.23}};
    StructFieldsConvert(parentStruct, sharedWriter, parentMappingTuple);
    
    // 验证数据
    const uint8_t* data = sharedWriter->data();
    ASSERT_NE(data, nullptr);
    
    // 预期结构：外层TLV包含键值 + 内部的两个TLV
    // 内部数据大小：(int TLV: 4+4+4) + (double TLV: 4+4+8) = 28 字节
    // 键值长度：strlen(SUB_STRUCT_KEY_NAME) + 1 = 8 字节
    // 外层数据大小：4+4+(8+28) = 44 字节
    size_t expectedInnerSize = 2 * (2 * sizeof(uint32_t)) + sizeof(int32_t) + sizeof(double);
    size_t keyLen = strlen(SUB_STRUCT_KEY_NAME) + 1;
    size_t expectedOuterSize = 2 * sizeof(uint32_t) + keyLen + expectedInnerSize;
    EXPECT_EQ(sharedWriter->size(), expectedOuterSize);
    
    // 验证外层 TLV
    size_t offset = 0;
    uint32_t actualType;
    memcpy(&actualType, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualType, SUB_STRUCT_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t actualLength;
    memcpy(&actualLength, data + offset, sizeof(uint32_t));
    EXPECT_EQ(actualLength, keyLen + expectedInnerSize);
    offset += sizeof(uint32_t);
    
    // 验证键值部分
    EXPECT_EQ(memcmp(data + offset, SUB_STRUCT_KEY_NAME, keyLen), 0);
    offset += keyLen;
    
    // 验证内层第一个 TLV（int）
    uint32_t innerType1;
    memcpy(&innerType1, data + offset, sizeof(uint32_t));
    EXPECT_EQ(innerType1, SUB_INT_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t innerLength1;
    memcpy(&innerLength1, data + offset, sizeof(uint32_t));
    EXPECT_EQ(innerLength1, sizeof(int32_t));
    offset += sizeof(uint32_t);
    
    int32_t actualIntValue;
    memcpy(&actualIntValue, data + offset, sizeof(int32_t));
    EXPECT_EQ(actualIntValue, parentStruct.subData.intField);
    offset += sizeof(int32_t);
    
    // 验证内层第二个 TLV（double）
    uint32_t innerType2;
    memcpy(&innerType2, data + offset, sizeof(uint32_t));
    EXPECT_EQ(innerType2, SUB_DOUBLE_TYPE);
    offset += sizeof(uint32_t);
    
    uint32_t innerLength2;
    memcpy(&innerLength2, data + offset, sizeof(uint32_t));
    EXPECT_EQ(innerLength2, sizeof(double));
    offset += sizeof(uint32_t);
    
    double actualDoubleValue;
    memcpy(&actualDoubleValue, data + offset, sizeof(double));
    EXPECT_DOUBLE_EQ(actualDoubleValue, parentStruct.subData.doubleField);
}

#if 0
using IntArray10 = uint32_t[10];

// 测试用于 uint32_t 数组元素的变长数组结构体
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(TestVariableLengthUint32Array,
    (uint32_t, arrayCount),
    (IntArray10, uint32Array)
);

// 测试用于结构体数组元素的简洁结构体
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(SimpleElement,
    (int32_t, id),
    (uint32_t, value)
);

using SimpleElementArray5 = SimpleElement[5];

DEFINE_STRUCT_WITH_TUPLE_INTERFACE(TestVariableLengthStructArray,
    (uint32_t, elementCount),
    (SimpleElementArray5, elements)
);

// 测试变长数组 TLV 转换器 - uint32_t 数组元素
TEST_F(TLVWriterTest, VariableLengthArrayTLVConverter_Uint32Array) {
    constexpr uint32_t UINT32_ARRAY_TYPE = 0xB001;
    
    auto sharedWriter = std::shared_ptr<TLVWriter>(std::move(writer));

    auto mappingTuple = MakeMappingRuleTuple(
        MAKE_TLV_VARIABLE_LENGTH_ARRAY_MAPPING(MakeFieldPath<1>(), 10, UINT32_ARRAY_TYPE)
    );

    TestVariableLengthUint32Array testStruct{3, {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000}};
    StructFieldsConvert(testStruct, sharedWriter, mappingTuple);
    
    // 验证数据
    const uint8_t* data = sharedWriter->data();
    ASSERT_NE(data, nullptr);
    
    // 预期：3个 uint32_t 元素，每个元素一个 TLV
    // 每个TLV: 4字节(type) + 4字节(length) + 4字节(value) = 12字节
    // 总共: 3 * 12 = 36字节
    size_t expectedSize = 3 * (2 * sizeof(uint32_t) + sizeof(uint32_t));
    EXPECT_EQ(sharedWriter->size(), expectedSize);
    
    // 验证每个数组元素的 TLV
    size_t offset = 0;
    uint32_t expectedValues[] = {100, 200, 300};
    
    for (int i = 0; i < 3; i++) {
        uint32_t actualType;
        memcpy(&actualType, data + offset, sizeof(uint32_t));
        EXPECT_EQ(actualType, UINT32_ARRAY_TYPE);
        offset += sizeof(uint32_t);
        
        uint32_t actualLength;
        memcpy(&actualLength, data + offset, sizeof(uint32_t));
        EXPECT_EQ(actualLength, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        
        uint32_t actualValue;
        memcpy(&actualValue, data + offset, sizeof(uint32_t));
        EXPECT_EQ(actualValue, expectedValues[i]);
        offset += sizeof(uint32_t);
    }
}

// 测试变长数组 TLV 转换器 - 结构体数组元素
TEST_F(TLVWriterTest, VariableLengthArrayTLVConverter_StructArray) {
    constexpr uint32_t STRUCT_ARRAY_TYPE = 0xB002;
    
    auto sharedWriter = std::shared_ptr<TLVWriter>(std::move(writer));

    auto mappingTuple = MakeMappingRuleTuple(
        MAKE_TLV_VARIABLE_LENGTH_ARRAY_MAPPING(MakeFieldPath<1>(), MakeFieldPath<0>(), STRUCT_ARRAY_TYPE)
    );

    TestVariableLengthStructArray testStruct{2, {{1, 100}, {2, 200}, {3, 300}, {4, 400}, {5, 500}}};
    StructFieldsConvert(testStruct, sharedWriter, mappingTuple);
    
    // 验证数据
    const uint8_t* data = sharedWriter->data();
    ASSERT_NE(data, nullptr);
    
    // 预期：2个 SimpleElement 结构体，每个结构体一个 TLV
    // 每个TLV: 4字节(type) + 4字节(length) + 8字节(SimpleElement) = 16字节
    // 总共: 2 * 16 = 32字节
    size_t expectedSize = 2 * (2 * sizeof(uint32_t) + sizeof(SimpleElement));
    EXPECT_EQ(sharedWriter->size(), expectedSize);
    
    // 验证每个数组元素的 TLV
    size_t offset = 0;
    SimpleElement expectedElements[] = {{1, 100}, {2, 200}};
    
    for (int i = 0; i < 2; i++) {
        uint32_t actualType;
        memcpy(&actualType, data + offset, sizeof(uint32_t));
        EXPECT_EQ(actualType, STRUCT_ARRAY_TYPE);
        offset += sizeof(uint32_t);
        
        uint32_t actualLength;
        memcpy(&actualLength, data + offset, sizeof(uint32_t));
        EXPECT_EQ(actualLength, sizeof(SimpleElement));
        offset += sizeof(uint32_t);
        
        SimpleElement actualElement;
        memcpy(&actualElement, data + offset, sizeof(SimpleElement));
        EXPECT_EQ(actualElement.id, expectedElements[i].id);
        EXPECT_EQ(actualElement.value, expectedElements[i].value);
        offset += sizeof(SimpleElement);
    }
}
#endif