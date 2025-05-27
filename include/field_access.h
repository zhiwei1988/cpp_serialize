#pragma once

#include <tuple>
#include <utility>
#include <type_traits>

namespace csrl {

// 自定义实现C++17才支持的std::void_t
template <typename...>
struct void_t_impl {
    using type = void;
};

template <typename... Ts>
using void_t = typename void_t_impl<Ts...>::type;

template <std::size_t I, typename T> 
auto GetField(T&& obj) -> decltype(std::forward<T>(obj).template GetField<I>())
{
    return std::forward<T>(obj).template GetField<I>();
}

// 定义类型 T 的第 I 个字段类型
template <std::size_t I, typename T>
using FieldType = std::decay_t<decltype(GetField<I>(std::declval<T&>()))>;

// 检测聚合类是否可以被类 Tuple 接口访问
template <typename T, typename = void> 
struct IsVisitable : std::false_type
{
};

template <typename T>
struct IsVisitable<T, csrl::void_t<decltype(std::declval<T>().template GetField<0>()), decltype(T::FieldCount)>>
    : std::true_type
{
};

// 使聚合类型可被类 Tuple 接口访问
#define MAKE_VISITABLE(...) \
    template <std::size_t N> \
    auto& GetField() { \
        return std::get<N>(std::tie(__VA_ARGS__)); \
    } \
    template <std::size_t N> \
    const auto& GetField() const { \
        return std::get<N>(std::tie(__VA_ARGS__)); \
    } \
    static constexpr std::size_t FieldCount = \
        std::tuple_size<decltype(std::tie(__VA_ARGS__))>::value;

}