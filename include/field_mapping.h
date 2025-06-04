#pragma once

#include <tuple>
#include <utility>
#include <type_traits>
#include <functional>
#include <cstring>

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
struct FieldMappingCustomRule : public FieldMappingRule<SrcIdx, DstIdx, Func>
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
struct StructFieldMappingRule : public FieldMappingRule<SrcIdx, DstIdx, void>
{
    RuleTuple ruleTuple;

    explicit StructFieldMappingRule(RuleTuple _ruleTuple) : ruleTuple(std::move(_ruleTuple)) {}

    template <typename SrcStruct, typename DstStruct> 
    void Convert(SrcStruct& src, DstStruct& dst) const
    {
        StructFieldsConvert(src, dst, ruleTuple);
    }
};

// std::string 到 char[N] 的映射规则
template <std::size_t SrcIdx, std::size_t DstIdx>
struct StringToCharArrayMappingRule : public FieldMappingRule<SrcIdx, DstIdx, void>
{
    // 从 std::string 到 char[N] 的转换
    template <typename SrcType, typename DstType>
    void Convert(SrcType& src, DstType& dst) const
    {
        static_assert(std::is_same<SrcType, std::string>::value, "Source must be std::string");
        static_assert(std::is_array<DstType>::value, "Destination must be an array");
        static_assert(std::is_same<typename std::remove_extent<DstType>::type, char>::value, "Destination array must be of char type");

        constexpr std::size_t N = std::extent<DstType>::value;

        strncpy(dst, src.c_str(), N - 1);
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
auto MakeFieldMappingRule()
{
    return FieldMappingRule<SrcIdx, DstIdx>{};
}

// 创建带自定义转换器的字段映射规则
template <std::size_t SrcIdx, std::size_t DstIdx, typename Func>
auto MakeFieldMappingCustomRule(Func&& converter)
{
    return FieldMappingCustomRule<SrcIdx, DstIdx, std::decay_t<Func>>(std::forward<Func>(converter));
}

// 创建结构体类型字段映射规则
template <std::size_t SrcIdx, std::size_t DstIdx, typename RuleTuple>
auto MakeStructFieldMappingRule(RuleTuple&& ruleTuple)
{
    return StructFieldMappingRule<SrcIdx, DstIdx, std::decay_t<RuleTuple>>(std::forward<RuleTuple>(ruleTuple));
}

// 创建 std::string 到 char[N] 的映射规则
template <std::size_t SrcIdx, std::size_t DstIdx>
auto MakeStringToCharArrayMappingRule()
{
    return StringToCharArrayMappingRule<SrcIdx, DstIdx>{};
}

// 创建映射规则集合
template <typename... MappingsRules>
auto MakeMappingRuleTuple(MappingsRules&&... mappingsRules)
{
    return MappingRuleTuple<std::decay_t<MappingsRules>...>{std::forward<MappingsRules>(mappingsRules)...};
}

} // namespace csrl