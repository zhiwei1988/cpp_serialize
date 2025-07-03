#pragma once

#include <tuple>
#include <utility>
#include <type_traits>
#include <functional>
#include <cstring>
#include "define_type_traits.h"

namespace csrl {

// 将 std::string 转换为 char[N] 的转换函数
template<typename SrcType, typename DstType>
void StringToCharArrayConverter(const SrcType& src, DstType& dst) 
{
    static_assert(std::is_same<SrcType, std::string>::value, "Source must be std::string");
    static_assert(std::is_array<DstType>::value, "Destination must be an array");
    static_assert(std::is_same<typename std::remove_extent<DstType>::type, char>::value, "Destination must be char array");
    
    constexpr std::size_t N = std::extent<DstType>::value;

    strncpy(dst, src.c_str(), N - 1);
}

// 将 char[N] 转换为 std::string 的转换函数
template<typename SrcType, typename DstType>
void CharArrayToStringConverter(const SrcType& src, DstType& dst)
{
    static_assert(std::is_array<SrcType>::value, "Source must be an array");
    static_assert(std::is_same<typename std::remove_extent<SrcType>::type, char>::value, "Source must be char array");
    static_assert(std::is_same<DstType, std::string>::value, "Destination must be std::string");

    dst = std::string(src);
}

// 表示字段访问路径的模板
template<std::size_t... Index>
struct FieldPath {
    static constexpr std::size_t depth = sizeof...(Index);
    static constexpr bool is_empty = (depth == 0);
};

template<std::size_t... Indexs>
constexpr std::size_t FieldPath<Indexs...>::depth;

template<std::size_t... Indexs>
constexpr bool FieldPath<Indexs...>::is_empty;

// 这里不能使用函数模版，因为函数模版不能被偏特化，函数模版重载时会产生歧义
template<std::size_t FirstIndex, std::size_t... RestIndexs>
struct PathAccessor {
    template<typename S>
    static decltype(auto) GetField(S& s) {
        return PathAccessor<RestIndexs...>::GetField(std::get<FirstIndex>(s));
    }
};

// 特化：路径访问的终止条件
template<std::size_t LastIndex>
struct PathAccessor<LastIndex> {
    template<typename S>
    static decltype(auto) GetField(S& s) {
        return std::get<LastIndex>(s);
    }
};

template<typename Struct, std::size_t... Indices>
decltype(auto) GetFieldByPath(Struct& s, FieldPath<Indices...>)
{
    return PathAccessor<Indices...>::GetField(s);
}

// 特化：空路径直接返回原对象本身
template<typename Struct>
decltype(auto) GetFieldByPath(Struct& s, FieldPath<>) 
{
    return s;
}

template <typename SrcPath, typename DstPath, typename Converter = void> 
struct FieldMappingRule {
    using SrcPathType = SrcPath;
    using DstPathType = DstPath;
    using ConverterType = Converter;
};

// 特化内置类型字段默认映射规则（直接赋值）
template <typename SrcPath, typename DstPath> 
struct FieldMappingRule<SrcPath, DstPath, void>
{
    // 默认转换器
    template <typename SrcType, typename DstType> 
    static void Convert(SrcType& src, DstType& dst)
    {
        GetFieldByPath(dst, DstPath{}) = 
            static_cast<csrl::remove_cvref_t<decltype(GetFieldByPath(dst, DstPath{}))>>(GetFieldByPath(src, SrcPath{}));
    }
};

// 特化内置类型字段自定义映射规则
template <typename SrcPath, typename DstPath, typename Func>
struct FieldMappingCustomRule : public FieldMappingRule<SrcPath, DstPath, Func>
{
    Func converter;

    explicit FieldMappingCustomRule(Func f) : converter(std::move(f)) {}

    template <typename SrcType, typename DstType> 
    void Convert(SrcType& src, DstType& dst) const
    {
        converter(GetFieldByPath(src, SrcPath{}), GetFieldByPath(dst, DstPath{}));
    }
};

// 前向声明
template <typename SrcStruct, typename DstStruct, typename MappingRuleTuple>
void StructFieldsConvert(SrcStruct& src, DstStruct& dst, const MappingRuleTuple& mappingRuleTuple);

// 特化结构体类型字段映射规则
template <typename SrcPath, typename DstPath, typename RuleTuple>
struct StructFieldMappingRule : public FieldMappingRule<SrcPath, DstPath, void>
{
    RuleTuple ruleTuple;

    explicit StructFieldMappingRule(RuleTuple _ruleTuple) : ruleTuple(std::move(_ruleTuple)) {}

    template <typename SrcStruct, typename DstStruct> 
    void Convert(SrcStruct& src, DstStruct& dst) const
    {
        StructFieldsConvert(GetFieldByPath(src, SrcPath{}), GetFieldByPath(dst, DstPath{}), ruleTuple);
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

template <std::size_t... Indexs>
constexpr FieldPath<Indexs...> MakeFieldPath() {
    return FieldPath<Indexs...>{};
}

// 创建默认字段映射规则
template <std::size_t... SrcIndexs, std::size_t... DstIndexs>
auto MakeFieldMappingRule(FieldPath<SrcIndexs...>, FieldPath<DstIndexs...>)
{
    return FieldMappingRule<FieldPath<SrcIndexs...>, FieldPath<DstIndexs...>>{};
}

// 创建带自定义转换器的字段映射规则
template <std::size_t... SrcIndexs, std::size_t... DstIndexs, typename Func>
auto MakeFieldMappingCustomRule(FieldPath<SrcIndexs...>, FieldPath<DstIndexs...>, Func&& converter)
{
    return FieldMappingCustomRule<FieldPath<SrcIndexs...>, FieldPath<DstIndexs...>, std::decay_t<Func>>(std::forward<Func>(converter));
}

// 创建结构体类型字段映射规则
template <std::size_t... SrcIndexs, std::size_t... DstIndexs, typename RuleTuple>
auto MakeStructFieldMappingRule(FieldPath<SrcIndexs...>, FieldPath<DstIndexs...>, RuleTuple&& ruleTuple)
{
    return StructFieldMappingRule<FieldPath<SrcIndexs...>, FieldPath<DstIndexs...>, std::decay_t<RuleTuple>>(std::forward<RuleTuple>(ruleTuple));
}

// 创建映射规则集合
template <typename... MappingsRules>
auto MakeMappingRuleTuple(MappingsRules&&... mappingsRules)
{
    return MappingRuleTuple<std::decay_t<MappingsRules>...>{std::forward<MappingsRules>(mappingsRules)...};
}

} // namespace csrl