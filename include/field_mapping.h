#pragma once

#include <tuple>
#include <utility>
#include <type_traits>
#include <functional>

namespace csrl {

template <std::size_t SrcIdx, std::size_t DstIdx, typename Converter = void> 
struct FieldMapping {
    static constexpr std::size_t srcIndex = SrcIdx;
    static constexpr std::size_t dstIndex = DstIdx;
    using ConverterType = Converter;
};

// 特化默认转换器（直接赋值）
template <std::size_t SrcIdx, std::size_t DstIdx> 
struct FieldMapping<SrcIdx, DstIdx, void>
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

template <std::size_t SrcIdx, std::size_t DstIdx, typename Func>
struct CustomFieldMapping : FieldMapping<SrcIdx, DstIdx, Func>
{
    Func converter;

    explicit CustomFieldMapping(Func f) : converter(std::move(f)) {}

    template <typename SrcType, typename DstType> 
    void Convert(SrcType& src, DstType& dst) const
    {
        converter(src, dst);
    }
};

// 映射集合
template <typename... FieldMappings> 
struct FieldMappingTuple
{
    std::tuple<FieldMappings...> mappings;
    
    FieldMappingTuple(FieldMappings&&... maps) : mappings(std::forward<FieldMappings>(maps)...) {}
    
    static constexpr std::size_t size = sizeof...(FieldMappings);
    
    template <std::size_t I>
    const auto& GetMapping() const {
        return std::get<I>(mappings);
    }
};

// 创建默认字段映射
template <std::size_t SrcIdx, std::size_t DstIdx>
constexpr auto DefaultFieldMapping()
{
    return FieldMapping<SrcIdx, DstIdx>{};
}

// 创建带自定义转换器的字段映射
template <std::size_t SrcIdx, std::size_t DstIdx, typename Func>
constexpr auto FieldMappingWith(Func&& converter)
{
    return CustomFieldMapping<SrcIdx, DstIdx, std::decay_t<Func>>(std::forward<Func>(converter));
}

template <typename... FieldMappings>
constexpr auto MakeFieldMappingTuple(FieldMappings&&... fieldMappings)
{
    return FieldMappingTuple<std::decay_t<FieldMappings>...>{std::forward<FieldMappings>(fieldMappings)...};
}

} // namespace csrl