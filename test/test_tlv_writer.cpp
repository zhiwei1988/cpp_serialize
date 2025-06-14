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
#include <tlv_writer.h>
#include <cstring>
#include <memory>
#include "define_tuple_interface.h"
#include "field_mapping.h"
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

DEFINE_STRUCT_WITH_TUPLE_INTERFACE(TestArithmeticTLVConverter,
    (int32_t, intValue),
    (double, doubleValue),
    (uint32_t, uint32Value)
);

// 测试算术类型 TLV 转换器
TEST_F(TLVWriterTest, ArithmeticTLVConverter_Basic) {
    constexpr uint32_t INT_TYPE = 0x6001;
    constexpr uint32_t DOUBLE_TYPE = 0x6002;
    constexpr uint32_t UINT32_TYPE = 0x6003;
    
    auto sharedWriter = std::shared_ptr<TLVWriter>(std::move(writer));

    auto mappingTuple = MakeMappingRuleTuple(
        MAKE_TLV_ARITHMETIC_MAPPING(MakeFieldPath<0>(), INT_TYPE),
        MAKE_TLV_ARITHMETIC_MAPPING(MakeFieldPath<1>(), DOUBLE_TYPE),
        MAKE_TLV_ARITHMETIC_MAPPING(MakeFieldPath<2>(), UINT32_TYPE)
    );

    TestArithmeticTLVConverter testArithmeticTLVConverter{12345, 3.14159, 12345};
    StructFieldsConvert(testArithmeticTLVConverter, sharedWriter, mappingTuple);
    
    // 验证数据
    const uint8_t* data = sharedWriter->data();
    ASSERT_NE(data, nullptr);
    
    size_t expectedSize = 3 * (2 * sizeof(uint32_t)) + sizeof(int32_t) + sizeof(double) + sizeof(uint32_t);
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
}
#endif