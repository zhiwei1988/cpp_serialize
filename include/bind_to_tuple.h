#pragma once

#include <tuple>
#include <type_traits>

namespace csrl {
// 辅助函数模板，使用索引序列来构建指针元组
// T 是要查看的对象的类型，Is 是索引序列 0, 1, ..., N-1
template <typename T, std::size_t... is>
auto MakeViewImpl(T& t, std::index_sequence<is...>)
{
    return std::make_tuple(&std::get<is>(t)...);
}

// N > 0 的通用模板定义
template <std::size_t n>
struct TupleViewHelper
{
    // auto& 参数类型使 TupleView 成为一个函数模板
    template <typename T>
    static auto TupleView(T& _t) { return MakeViewImpl(_t, std::make_index_sequence<n>{}); }
};

} // namespace csrl