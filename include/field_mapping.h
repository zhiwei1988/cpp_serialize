#pragma once

#include <tuple>
#include <utility>
#include <type_traits>
#include <functional>

namespace csrl {

template <std::size_t SrcIdx, std::size_t DstIdx, typename Converter = void> 
struct FieldMappingRule {
    static constexpr std::size_t srcIndex = SrcIdx;
    static constexpr std::size_t dstIndex = DstIdx;
    using ConverterType = Converter;
};

// 特化内置类型字段默认映射规则（直接赋值）
template <std::size_t SrcIdx, std::size_t DstIdx> 
struct FieldMappingRule<SrcIdx, DstIdx, void>
{
    static constexpr std::size_t srcIndex = SrcIdx;
    static constexpr std::size_t dstIndex = DstIdx;

    // 默认转换器
    template <typename SrcType, typename DstType> 
    static void Convert(SrcType& src, DstType& dst)
    {
        dst = static_cast<DstType>(src);
    }
};

// 特化内置类型字段自定义映射规则
template <std::size_t SrcIdx, std::size_t DstIdx, typename Func>
struct FieldMappingCustomRule : FieldMappingRule<SrcIdx, DstIdx, Func>
{
    Func converter;

    explicit FieldMappingCustomRule(Func f) : converter(std::move(f)) {}

    template <typename SrcType, typename DstType> 
    void Convert(SrcType& src, DstType& dst) const
    {
        converter(src, dst);
    }
};

// 前向声明
template <typename SrcStruct, typename DstStruct, typename MappingRuleTuple>
void StructFieldsConvert(SrcStruct& src, DstStruct& dst, const MappingRuleTuple& mappingRuleTuple);

// 特化结构体类型字段映射规则
template <std::size_t SrcIdx, std::size_t DstIdx, typename RuleTuple>
struct StructFieldMappingRule : FieldMappingRule<SrcIdx, DstIdx, void>
{
    RuleTuple ruleTuple;

    explicit StructFieldMappingRule(RuleTuple _ruleTuple) : ruleTuple(std::move(_ruleTuple)) {}

    template <typename SrcStruct, typename DstStruct> 
    void Convert(SrcStruct& src, DstStruct& dst) const
    {
        StructFieldsConvert(src, dst, ruleTuple);
    }
};

// 映射集合
template <typename... MappingsRules> 
struct MappingRuleTuple
{
    std::tuple<MappingsRules...> mappings;
    
    MappingRuleTuple(MappingsRules&&... maps) : mappings(std::forward<MappingsRules>(maps)...) {}
    
    static constexpr std::size_t size = sizeof...(MappingsRules);
    
    template <std::size_t I>
    const auto& GetMapping() const {
        return std::get<I>(mappings);
    }
};

// 创建默认字段映射规则
template <std::size_t SrcIdx, std::size_t DstIdx>
constexpr auto MakeFieldMappingRule()
{
    return FieldMappingRule<SrcIdx, DstIdx>{};
}

// 创建带自定义转换器的字段映射规则
template <std::size_t SrcIdx, std::size_t DstIdx, typename Func>
constexpr auto MakeFieldMappingCustomRule(Func&& converter)
{
    return FieldMappingCustomRule<SrcIdx, DstIdx, std::decay_t<Func>>(std::forward<Func>(converter));
}

// 创建结构体类型字段映射规则
template <std::size_t SrcIdx, std::size_t DstIdx, typename RuleTuple>
constexpr auto MakeStructFieldMappingRule(RuleTuple&& ruleTuple)
{
    return StructFieldMappingRule<SrcIdx, DstIdx, std::decay_t<RuleTuple>>(std::forward<RuleTuple>(ruleTuple));
}

// 创建映射规则集合
template <typename... MappingsRules>
constexpr auto MakeMappingRuleTuple(MappingsRules&&... mappingsRules)
{
    return MappingRuleTuple<std::decay_t<MappingsRules>...>{std::forward<MappingsRules>(mappingsRules)...};
}

} // namespace csrl