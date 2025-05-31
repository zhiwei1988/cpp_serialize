#pragma once

#include "field_access.h"
#include "field_mapping.h"
#include "define_tuple_interface.h"

namespace csrl {
template <typename SrcStruct, typename DstStruct, typename MappingRuleTuple, std::size_t I> 
void SingleFieldConvert(SrcStruct& src, DstStruct& dst, const MappingRuleTuple& mappingRuleTuple)
{
    const auto& mappingRule = mappingRuleTuple.template GetMapping<I>();
    using MappingRuleType = std::remove_cv_t<std::remove_reference_t<decltype(mappingRule)>>;
    constexpr std::size_t srcIndex = MappingRuleType::srcIndex;
    constexpr std::size_t dstIndex = MappingRuleType::dstIndex;

    mappingRule.Convert(std::get<srcIndex>(src), std::get<dstIndex>(dst));
}

template <typename SrcStruct, typename DstStruct, typename MappingRuleTuple, std::size_t... I>
void ConvertAllFields(SrcStruct& src, DstStruct& dst, const MappingRuleTuple& mappingRuleTuple, std::index_sequence<I...>)
{
    // 展开所有映射项进行转换 (C++17中可以用fold expression简化)
    int dummy[] = {0, (SingleFieldConvert<SrcStruct, DstStruct, MappingRuleTuple, I>(src, dst, mappingRuleTuple), 0)...};
    (void)dummy; // 避免未使用变量警告
}

// 主转换器
template <typename SrcStruct, typename DstStruct, typename MappingRuleTuple>
void StructFieldsConvert(SrcStruct& src, DstStruct& dst, const MappingRuleTuple& mappingRuleTuple)
{
    ConvertAllFields(src, dst, mappingRuleTuple, std::make_index_sequence<MappingRuleTuple::size>{});
}
} // namespace csrl