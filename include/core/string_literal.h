#pragma once

#include <string>
#include <array>
#include <algorithm>

namespace csrl {

// 一个编译时字符串字面量模板结构。
// 它的主要作用是解决普通字符串无法作为非类型模板参数的问题。
template <std::size_t N>
struct StringLiteral {
private:
    // std::array 的非 const operator[] 直到 C++17 才被标记为 constexpr。
    // 辅助函数：在编译期从 C 风格字符串字面量创建 std::array
    template <std::size_t... Is>
    static constexpr std::array<char, N> create_array_from_literal(const char (&s)[N], std::index_sequence<Is...>) {
        // 内部的 {s[Is]...} 创建一个 char 数组的初始化列表
        // 外部的 {} 将此列表用于 std::array 的聚合初始化
        return { {s[Is]...} };
    }

public:
    constexpr StringLiteral(const char (&_str)[N]) 
    {
        arr_ = create_array_from_literal(_str, std::make_index_sequence<N>());
    }

    std::string str() const { return std::string(arr_.data(), N - 1); }

    constexpr std::size_t size() const {
        return N - 1;
    }

    constexpr const char* data() const {
        return arr_.data();
    }

    constexpr char operator[](std::size_t index) const {
        return arr_[index];
    }

    std::array<char, N> arr_{};
};

// 由于 C++14 不支持类模版参数推导，所以需要提供一个辅助函数来创建 StringLiteral 对象
template <std::size_t CharArraySize>
constexpr StringLiteral<CharArraySize> make_string_literal(const char (&str)[CharArraySize]) 
{
    return StringLiteral<CharArraySize>(str);
}

template <std::size_t N1, std::size_t N2>
constexpr inline bool operator==(const StringLiteral<N1>& first,
                                 const StringLiteral<N2>& second) 
{
  return first.str() == second.str();
}

template <std::size_t N1, std::size_t N2>
constexpr inline bool operator!=(const StringLiteral<N1>& first,
                                 const StringLiteral<N2>& second)
{
  return !(first == second);
}

} // namespace csrl