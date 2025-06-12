#pragma once

#include <cstddef>
#include <type_traits>

namespace csrl {

// 当 any 结构体用于像 SomeType{any(0), any(1)} 这样的表达式时：
// any(0) 和 any(1) 会调用 any(std::size_t) 构造函数创建临时的 any 对象。
// 当编译器尝试匹配 SomeType 的构造函数参数（或其聚合初始化成员）时，它会发现提供的参数是 any类型的。
// 如果 SomeType 的第一个构造参数（或成员）期望类型 Arg1Type，编译器会查找 any 是否能转换为 Arg1Type。这时，模板化的类型转换运算符 any::operator T() 会被实例化为 any::operator Arg1Type()。
// 如果这个转换运算符的定义（例如 return Arg1Type{};）是有效的，那么转换成功，any(0) 就提供了 Arg1Type 类型的参数。
// 对后续的 any(1) 和其他参数也是如此。
struct any {
    any(std::size_t);
    // 定义类型转换运算符
    template <typename T> constexpr operator T() const noexcept;
};

template <typename T>
struct FieldsCountHelper {
private:
    template <std::size_t n>
    struct ConstructibleHelper {
        // 检查 U 是否可以通过 n 个参数构造
        // 如果可以，则返回 std::true_type
        template <typename U, std::size_t... is>
        static auto Check(std::index_sequence<is...>) -> decltype(U{any(is)...}, std::true_type{});

        // 如果不能通过 n 个参数构造，则返回 std::false_type
        template <typename U> static auto Check(...) -> std::false_type;

        // 返回检查结果
        static constexpr bool result = decltype(Check<T>(std::make_index_sequence<n>{}))::value;
  };

    // 基础模板：默认情况
    template <std::size_t n, bool = ConstructibleHelper<n>::result>
    struct CountMaxArgsInAggInitHelper {
        static constexpr std::size_t value = n;
    };

    // 特化1：当类型可以用 n 个参数构造时
    // 用于递归查找最大参数数量的辅助结构体
    template <std::size_t n>
    struct CountMaxArgsInAggInitHelper<n, true> {
        static constexpr std::size_t value = CountMaxArgsInAggInitHelper<n + 1>::value;
    };

    // 特化2：当类型不能用 n 个参数构造时
    template <std::size_t n>
    struct CountMaxArgsInAggInitHelper<n, false> {
        static constexpr std::size_t value = n - 1;
    };

public:
    template <std::size_t n>
    static constexpr bool Constructible()
    {
        return ConstructibleHelper<n>::result;
    }

    static constexpr std::size_t CountMaxArgsInAggInit()
    {
        return CountMaxArgsInAggInitHelper<0>::value;
    }

    // 约束：T 中不能包含 C 风格的数组
    // 建议使用 std::array 来替代 C 风格的数组
    static constexpr std::size_t CountFields()
    {
        constexpr std::size_t maxAggArgs = CountMaxArgsInAggInit();
        return maxAggArgs;
    }
};

template <class T>
constexpr std::size_t FieldsCounts = FieldsCountHelper<T>::CountFields();

}
