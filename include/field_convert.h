#pragma once

#include "field_access.h"
#include "field_mapping.h"
#include "define_tuple_interface.h"

namespace csrl {
template <typename SrcStruct, typename DstStruct, std::size_t I> 
struct SingleFieldConverter {
    template <typename FieldMapTuple>
    static void Convert(SrcStruct& src, DstStruct& dst, const FieldMapTuple& mappingTuple)
    {
        const auto& fieldMap = mappingTuple.template GetMapping<I>();
        using FieldMapType = std::remove_cv_t<std::remove_reference_t<decltype(fieldMap)>>;
        constexpr std::size_t srcIndex = FieldMapType::srcIndex;
        constexpr std::size_t dstIndex = FieldMapType::dstIndex;

        fieldMap.Convert(std::get<srcIndex>(src), std::get<dstIndex>(dst));
    }
};

template <typename SrcStruct, typename DstStruct, typename MappingTuple, std::size_t... I>
void ConvertAllFields(SrcStruct& src, DstStruct& dst, const MappingTuple& mappingTuple, std::index_sequence<I...>)
{
    // 展开所有映射项进行转换 (C++17中可以用fold expression简化)
    int dummy[] = {0, (SingleFieldConverter<SrcStruct, DstStruct, I>::Convert(src, dst, mappingTuple), 0)...};
    (void)dummy; // 避免未使用变量警告
}

// 主转换器
template <typename SrcStruct, typename DstStruct, typename MappingTuple>
static void StructFieldsConvert(SrcStruct& src, DstStruct& dst, const MappingTuple& mappingTuple)
{
    ConvertAllFields<SrcStruct, DstStruct, MappingTuple>(src, dst, mappingTuple,
                                                         std::make_index_sequence<MappingTuple::size>{});
}
} // namespace csrl