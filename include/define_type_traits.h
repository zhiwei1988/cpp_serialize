#pragma once

#include <type_traits>

namespace csrl {

template <typename T>
struct remove_cvref {
    using type = typename std::remove_reference<typename std::remove_cv<T>::type>::type;
};

// 去除类型的常量（const）、易变（volatile）修饰符和引用
template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

} // namespace csrl