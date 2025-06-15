/**
 * @file tlv_writer.h
 * @author Zhiwei Tan (zhiweix1988@gmail.com)
 * @brief TLV 写入器，用于将数据序列化为 TLV 格式
 * @version 0.1
 * @date 2025-06-14 23:00:00
 * 
 * @copyright Copyright (c) 2025
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <utility>
#include <type_traits>
#include <vector>
#include "define_tuple_interface.h"
#include "field_mapping.h"
#include "field_convert.h"
#include "define_type_traits.h"

namespace csrl {

class TLVWriter {
public:
    explicit TLVWriter(size_t initialCapacity = 1024) { m_buffer.reserve(initialCapacity); }

    TLVWriter(const TLVWriter&) = delete;
    TLVWriter& operator=(const TLVWriter&) = delete;

    // AppendBuf 接口：仅传入一段数据
    int32_t AppendBuf(uint32_t type, const char* buf, size_t len) 
    { 
        return Append(type, buf, len); 
    }

    // AppendPair 接口：传入两段数据，内部会拼接两段数据
    int32_t AppendPair(uint32_t type, const char* firstBuf, size_t firstLen, const char* secBuf, size_t secLen) 
    {
        return Append(type, firstBuf, firstLen, secBuf, secLen);
    }

    size_t size() const { return m_buffer.size(); }
    const uint8_t* data() const { return m_buffer.data(); }
    void clear() { m_buffer.clear(); }

private:
    // 递归计算各段长度（要求参数必须成对：指针和长度）
    static size_t TotalLength() { return 0; }

    template <typename Ptr, typename Len, typename... Rest>
    static size_t TotalLength(const Ptr /*buf*/, Len len, Rest&&... rest) 
    {
        return len + TotalLength(std::forward<Rest>(rest)...);
    }

    // 辅助函数：将数据追加到 buffer
    void AppendToBuffer(const uint8_t* data, size_t len) 
    {
        if (len > 0 && data != nullptr) {
            m_buffer.insert(m_buffer.end(), data, data + len);
        }
    }

    // 递归写入各段数据到 buffer
    void AppendSegmentsToBuffer() {}

    template <typename Ptr, typename Len, typename... Rest>
    void AppendSegmentsToBuffer(const Ptr buf, Len len, Rest&&... rest) 
    {
        if (len > 0 && buf != nullptr) {
            const uint8_t* bytePtr = reinterpret_cast<const uint8_t*>(buf);
            m_buffer.insert(m_buffer.end(), bytePtr, bytePtr + len);
        }
        AppendSegmentsToBuffer(std::forward<Rest>(rest)...);
    }

    // 通用的追加接口：
    // 先写入 type (uint32_t) 和整个数据段的长度 (uint32_t)，再将各段数据拼接到一起
    template <typename... Args>
    int32_t Append(uint32_t type, Args&&... args) 
    {
        static_assert(sizeof...(Args) % 2 == 0, "Buffer segments must come in pairs: pointer and length.");
        
        // 先写入 type
        AppendToBuffer(reinterpret_cast<const uint8_t*>(&type), sizeof(type));
        
        // 计算并写入总长度
        size_t segmentsLen = TotalLength(std::forward<Args>(args)...);
        uint32_t valueLen = static_cast<uint32_t>(segmentsLen);
        AppendToBuffer(reinterpret_cast<const uint8_t*>(&valueLen), sizeof(valueLen));
        
        // 递归写入各段数据
        AppendSegmentsToBuffer(std::forward<Args>(args)...);
        
        return 0;
    }

    std::vector<uint8_t> m_buffer;
};

// TLV 转换器基类
template<uint32_t tlvType, const char* keyName = nullptr>
struct BaseTLVConverter {
    static constexpr uint32_t m_tlvType = tlvType;
    static constexpr const char* m_keyName = keyName;

    template<typename SrcType>
    void operator()(const SrcType& src, std::shared_ptr<TLVWriter>& dst) const 
    {
        if (m_keyName == nullptr) {
            dst->AppendBuf(m_tlvType, reinterpret_cast<const char*>(&src), sizeof(src));
        } else {
            dst->AppendPair(m_tlvType, m_keyName, strlen(m_keyName) + 1, reinterpret_cast<const char*>(&src), sizeof(src));
        }
    }

    // C 风格字符数组特化
    template<size_t N>
    void operator()(const char (&src)[N], std::shared_ptr<TLVWriter>& dst) const 
    {
        if (m_keyName == nullptr) {
            dst->AppendBuf(m_tlvType, src, strlen(src) + 1);
        } else {
            dst->AppendPair(m_tlvType, m_keyName, strlen(m_keyName) + 1, src, strlen(src) + 1);
        }
    }

    // C 风格非字符数组特化 (如 int[5])
    template<typename T, size_t N>
    typename std::enable_if<!std::is_same<T, char>::value, void>::type
    operator()(const T (&src)[N], std::shared_ptr<TLVWriter>& dst) const 
    {
        if (m_keyName == nullptr) {
            dst->AppendBuf(m_tlvType, reinterpret_cast<const char*>(src), sizeof(src));
        } else {
            dst->AppendPair(m_tlvType, m_keyName, strlen(m_keyName) + 1, 
                           reinterpret_cast<const char*>(src), sizeof(src));
        }
    }
};

// 数字转字符串 TLV 转换器
template <uint32_t TLVType, const char* KeyName = nullptr>
struct DigitalToStringTLVConverter : public BaseTLVConverter<TLVType, KeyName> {
    using BaseTLVConverter<TLVType, KeyName>::m_keyName;
    using BaseTLVConverter<TLVType, KeyName>::m_tlvType;

    template <typename SrcType>
    void operator()(const SrcType& src, std::shared_ptr<TLVWriter>& dst) const 
    {
        static_assert(std::is_integral<SrcType>::value, "DigitalToStringTLVConverter only works with integral types");
        std::string valueStr = std::to_string(src);
        if (m_keyName == nullptr) {
            dst->AppendBuf(m_tlvType, valueStr.c_str(), valueStr.length());
        } else {
            dst->AppendPair(m_tlvType, m_keyName, strlen(m_keyName) + 1, valueStr.c_str(), valueStr.length());
        }
    }
};

// 子结构体 TLV 转换器，用于子结构体中的成员需要逐个序列化的场景
// 子结构体如果作为一个整体进行序列化，则使用 BaseTLVConverter 转换器
template<uint32_t tlvType, typename RuleTuple, const char* keyName = nullptr>
struct SubStructTLVConverter : public BaseTLVConverter<tlvType, keyName> {
    using BaseTLVConverter<tlvType, keyName>::m_tlvType;
    using BaseTLVConverter<tlvType, keyName>::m_keyName;

    RuleTuple m_ruleTuple;
    
    explicit SubStructTLVConverter(RuleTuple &ruleTuple) : m_ruleTuple(std::move(ruleTuple)) {}
    
    template<typename SrcType>
    void operator()(SrcType& src, std::shared_ptr<TLVWriter>& dst) const 
    {
        auto tempDst = std::make_shared<TLVWriter>(1024);
        StructFieldsConvert(src, tempDst, m_ruleTuple);
        
        size_t len = tempDst->size();
        if (len > 0) {
            if (m_keyName == nullptr) {
                dst->AppendBuf(m_tlvType, reinterpret_cast<const char*>(tempDst->data()), len);
            } else {
                dst->AppendPair(m_tlvType, m_keyName, strlen(m_keyName) + 1, reinterpret_cast<const char*>(tempDst->data()), len);
            }
        }
    }
};

#if 0
template<uint32_t tlvType, size_t count>
struct VariableLengthArrayTLVConverter : public BaseTLVConverter<tlvType> {
    using BaseTLVConverter<tlvType>::m_tlvType;

    template<typename SrcType>
    void operator()(SrcType& src, std::shared_ptr<TLVWriter>& dst) const 
    {
        constexpr size_t arraySize = std::extent<remove_cvref_t<decltype(src)>>::value;
        size_t actualCount = (count < arraySize) ? count : arraySize;

        for (size_t i = 0; i < actualCount; i++) {
            dst->AppendBuf(m_tlvType, reinterpret_cast<const char*>(&src[i]), sizeof(src[i]));
        }
    }
};
#endif

template<typename SrcPath, typename ConverterType>
struct FieldMappingTLVCustomRule: public FieldMappingRule<SrcPath, std::shared_ptr<TLVWriter>, ConverterType> {
    ConverterType m_converter;

    explicit FieldMappingTLVCustomRule(ConverterType f) : m_converter(std::move(f)) {}

    template<typename SrcType>
    void Convert(SrcType& src, std::shared_ptr<TLVWriter>& dst) const
    {
        m_converter(GetFieldByPath(src, SrcPath{}), dst);
    }
};

template<std::size_t... SrcIndexs, typename ConverterType>
auto MakeFieldMappingTLVCustomRule(FieldPath<SrcIndexs...>, ConverterType&& converter)
{
    return FieldMappingTLVCustomRule<FieldPath<SrcIndexs...>, ConverterType>(std::forward<ConverterType>(converter));
}

// 新增某种特定的 TLV 转换器在此处添加宏

// 默认 TLV 转换器宏
#define MAKE_TLV_DEFAULT_MAPPING(SrcPath, TLVType) MakeFieldMappingTLVCustomRule(SrcPath, BaseTLVConverter<TLVType>{})

// 带键值的默认 TLV 转换器宏
#define MAKE_TLV_DEFAULT_MAPPING_WITH_KEY(SrcPath, TLVType, KeyName)                                                      \
    MakeFieldMappingTLVCustomRule(SrcPath, BaseTLVConverter<TLVType, KeyName>{})

// 数字转字符串 TLV 转换器宏
#define MAKE_TLV_DIGITAL_STRING_MAPPING(SrcPath, TLVType)                                                              \
    MakeFieldMappingTLVCustomRule(SrcPath, DigitalToStringTLVConverter<TLVType>{})

// 带键值的数字转字符串 TLV 转换器宏
#define MAKE_TLV_DIGITAL_STRING_MAPPING_WITH_KEY(SrcPath, TLVType, KeyName)                                                              \
    MakeFieldMappingTLVCustomRule(SrcPath, DigitalToStringTLVConverter<TLVType, KeyName>{})

// 子结构体 TLV 转换器宏
#define MAKE_TLV_SUB_STRUCT_MAPPING(SrcPath, TLVType, RuleTuple)                                                              \
    MakeFieldMappingTLVCustomRule(SrcPath, SubStructTLVConverter<TLVType, remove_cvref_t<decltype(RuleTuple)>>(RuleTuple))

// 带键值的子结构体 TLV 转换器宏
#define MAKE_TLV_SUB_STRUCT_MAPPING_WITH_KEY(SrcPath, TLVType, RuleTuple, KeyName)                                                              \
    MakeFieldMappingTLVCustomRule(SrcPath, SubStructTLVConverter<TLVType, remove_cvref_t<decltype(RuleTuple)>, KeyName>(RuleTuple))

#if 0
// 变长数组 TLV 转换器宏
#define MAKE_TLV_VARIABLE_LENGTH_ARRAY_MAPPING(SrcPath, Count, TLVType)                                                              \
    MakeFieldMappingTLVCustomRule(SrcPath, VariableLengthArrayTLVConverter<TLVType, Count>{})
#endif

}
