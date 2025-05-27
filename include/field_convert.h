#pragma once

#include "field_access.h"
#include "field_mapping.h"

namespace csrl {

#if 0
// 基本类型转换
template <typename SrcType, typename DstType, typename = void>
struct ValueConverter
{
    static void convert(const SrcType& src, DstType& dst) { dst = static_cast<DstType>(src); }
};

// 数组转换
template <typename SrcElem, typename DstElem, std::size_t N>
struct ValueConverter<SrcElem[N], DstElem[N]>
{
    static void Convert(const SrcElem (&src)[N], DstElem (&dst)[N])
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            ValueConverter<SrcElem, DstElem>::convert(src[i], dst[i]);
        }
    }
};

// 长度不一致数组转换 - 只拷贝共同部分
template <typename SrcElem, typename DstElem, std::size_t SN, std::size_t DN>
struct ValueConverter<SrcElem[SN], DstElem[DN]>
{
    static void convert(const SrcElem (&src)[SN], DstElem (&dst)[DN])
    {
        constexpr std::size_t min_size = SN < DN ? SN : DN;
        for (std::size_t i = 0; i < min_size; ++i)
        {
            ValueConverter<SrcElem, DstElem>::convert(src[i], dst[i]);
        }
    }
};

// 嵌套可访问结构体转换
template <typename SrcStruct, typename DstStruct>
struct ValueConverter<
    SrcStruct, DstStruct,
    std::enable_if_t<IsVisitable<SrcStruct>::value && IsVisitable<DstStruct>::value>>
{
    // 这里会有递归转换 - 实现会在后面提供
};
#endif

template <typename SrcStruct, typename DstStruct, std::size_t I>
struct SingleFieldConverter {
    template <typename FieldMapTuple>
    static void Convert(const SrcStruct& src, DstStruct& dst, const FieldMapTuple& mappingTuple)
    {
        const auto& fieldMap = mappingTuple.template GetMapping<I>();
        using FieldMapType = std::remove_cv_t<std::remove_reference_t<decltype(fieldMap)>>;
        constexpr std::size_t srcIndex = FieldMapType::srcIndex;
        constexpr std::size_t dstIndex = FieldMapType::dstIndex;

        fieldMap.Convert(GetField<srcIndex>(src), GetField<dstIndex>(dst));
    }
};

template <typename SrcStruct, typename DstStruct, typename MappingTuple, std::size_t... I>
void ConvertAllFields(const SrcStruct& src, DstStruct& dst, const MappingTuple& mappingTuple, std::index_sequence<I...>)
{
    // 展开所有映射项进行转换 (C++17中可以用fold expression简化)
    int dummy[] = {
        0,
        (SingleFieldConverter<SrcStruct, DstStruct, I>::Convert(src, dst, mappingTuple), 0)...};
    (void)dummy; // 避免未使用变量警告
}

// 主转换器
template <typename SrcStruct, typename DstStruct, typename MappingTuple>
struct StructConverter {
    static void Convert(const SrcStruct& src, DstStruct& dst, const MappingTuple& mappingTuple) 
    {
        ConvertAllFields<SrcStruct, DstStruct, MappingTuple>(src, dst, mappingTuple, std::make_index_sequence<MappingTuple::size>{});
    }
};
}