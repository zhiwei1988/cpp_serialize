#include <iostream>
#include "fields.h"
#include <string>
#include <cstddef>
#include <tuple>
#include "define_tuple_interface.h"
#include "field_convert.h"
#include "field_operator.h"
#include "tlv_writer.h"
#include <memory>

using CharArray = char[10];

DEFINE_STRUCT_WITH_TUPLE_INTERFACE(Test, (int, a), (int, b), (int, c))
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(Source, (int, id), (float, value), (Test, test), (std::string, name))
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(Destination, (float, val), (int, identifier), (Test, test), (CharArray, name))

using ArrayData = int[10];

// 新增：定义包含可变长数组的结构体
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(VariableArrayStruct, 
    (int, id),                    // 字段索引 0
    (uint32_t, arrayLength),      // 字段索引 1：数组长度
    (ArrayData, dataArray),         // 字段索引 2：可变长数组（最大10个元素）
    (float, weight)               // 字段索引 3
)

// 定义TLV类型常量
constexpr uint32_t TLV_TYPE_ID = 0x1001;
constexpr uint32_t TLV_TYPE_WEIGHT = 0x1002; 
constexpr uint32_t TLV_TYPE_ARRAY_DATA = 0x1003;

using namespace csrl;

void DemoVariableLengthArrayTLV()
{
    std::cout << "\n=== 可变长数组TLV序列化示例 ===\n";
    
    // 创建包含可变长数组的源数据
    VariableArrayStruct src{
        .id = 42,
        .arrayLength = 5,           // 实际使用5个元素
        .dataArray = {10, 20, 30, 40, 50, 0, 0, 0, 0, 0}, // 数组有10个位置，但只用前5个
        .weight = 3.14f
    };
    
    std::cout << "源数据:\n";
    std::cout << "  id = " << src.id << "\n";
    std::cout << "  arrayLength = " << src.arrayLength << "\n";
    std::cout << "  dataArray = [";
    for (uint32_t i = 0; i < src.arrayLength; ++i) {
        std::cout << src.dataArray[i];
        if (i < src.arrayLength - 1) std::cout << ", ";
    }
    std::cout << "]\n";
    std::cout << "  weight = " << src.weight << "\n\n";
    
    // 创建TLV Writer
    auto tlvWriter = std::make_shared<TLVWriter>(1024);
    
    // 创建TLV映射规则
    auto tlvMappingRules = MakeMappingRuleTuple(
        // 普通字段映射
        MAKE_TLV_DEFAULT_MAPPING(MakeFieldPath<0>(), TLV_TYPE_ID),
        MAKE_TLV_DEFAULT_MAPPING(MakeFieldPath<3>(), TLV_TYPE_WEIGHT),
        
        // 可变长数组映射 - 这是关键部分！
        // 参数说明：
        // - MakeFieldPath<>(): 整个结构体作为源路径
        // - 1: arrayLength字段的索引（长度字段）  
        // - 2: dataArray字段的索引（数组字段）
        // - TLV_TYPE_ARRAY_DATA: TLV类型
        MAKE_TLV_VARIABLE_LENGTH_ARRAY_MAPPING(MakeFieldPath<>(), 1, 2, TLV_TYPE_ARRAY_DATA)
    );
    
    // 执行TLV序列化
    StructFieldsConvert(src, tlvWriter, tlvMappingRules);
    
    // 输出序列化结果
    std::cout << "TLV序列化完成:\n";
    std::cout << "  总大小: " << tlvWriter->size() << " 字节\n";
    std::cout << "  数据内容: ";
    
    const uint8_t* data = tlvWriter->data();
    for (size_t i = 0; i < tlvWriter->size(); ++i) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) std::cout << "\n              ";
    }
    std::cout << "\n\n";
    
    // 分析TLV数据结构
    std::cout << "TLV数据分析:\n";
    size_t offset = 0;
    int tlvCount = 0;
    
    while (offset + 8 <= tlvWriter->size()) {
        uint32_t type = *reinterpret_cast<const uint32_t*>(data + offset);
        uint32_t length = *reinterpret_cast<const uint32_t*>(data + offset + 4);
        
        std::cout << "  TLV #" << ++tlvCount << ":\n";
        std::cout << "    Type: 0x" << std::hex << type << std::dec;
        
        switch (type) {
            case TLV_TYPE_ID:
                std::cout << " (ID)";
                break;
            case TLV_TYPE_WEIGHT:
                std::cout << " (WEIGHT)";
                break;  
            case TLV_TYPE_ARRAY_DATA:
                std::cout << " (ARRAY_DATA)";
                break;
            default:
                std::cout << " (UNKNOWN)";
        }
        
        std::cout << "\n    Length: " << length << " 字节\n";
        std::cout << "    Value: ";
        
        if (type == TLV_TYPE_ID && length == 4) {
            int value = *reinterpret_cast<const int*>(data + offset + 8);
            std::cout << value << " (整数)";
        } else if (type == TLV_TYPE_WEIGHT && length == 4) {
            float value = *reinterpret_cast<const float*>(data + offset + 8);
            std::cout << value << " (浮点数)";
        } else if (type == TLV_TYPE_ARRAY_DATA && length == 4) {
            int value = *reinterpret_cast<const int*>(data + offset + 8);
            std::cout << value << " (数组元素)";
        } else {
            for (uint32_t i = 0; i < length && i < 16; ++i) {
                printf("%02X ", data[offset + 8 + i]);
            }
            if (length > 16) std::cout << "...";
        }
        
        std::cout << "\n\n";
        offset += 8 + length;
    }
}

int main()
{
    // 原有的字段映射示例
    Source src{1, 2.0f, Test{1, 2, 3}, "hello"};
    Destination dst{0, 0, Test{0, 0, 0}, "world"};

    auto mappingTuple = MakeMappingRuleTuple(
        MakeFieldMappingRule(MakeFieldPath<1>(), MakeFieldPath<0>()),
        MakeFieldMappingRule(MakeFieldPath<0>(), MakeFieldPath<1>()),
        MakeStructFieldMappingRule(MakeFieldPath<2>(), MakeFieldPath<2>(),
                                   MakeMappingRuleTuple(MakeFieldMappingRule(MakeFieldPath<0>(), MakeFieldPath<0>()),
                                                        MakeFieldMappingRule(MakeFieldPath<1>(), MakeFieldPath<1>()),
                                                        MakeFieldMappingRule(MakeFieldPath<2>(), MakeFieldPath<2>()))),
        MakeFieldMappingCustomRule(MakeFieldPath<3>(), MakeFieldPath<3>(),
                                   csrl::StringToCharArrayConverter<std::string, CharArray>));

    csrl::StructFieldsConvert(src, dst, mappingTuple);
    std::cout << "基础字段映射结果: " << dst.val << " " << dst.identifier << " " << dst.test.a << " " << dst.test.b << " " << dst.test.c << " " << dst.name << std::endl;

    // 新增：可变长数组TLV序列化演示
    DemoVariableLengthArrayTLV();
    
    return 0;
}
