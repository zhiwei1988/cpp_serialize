/*
 * @Description: 在编译时从C++结构体中获取字段名
 */

#pragma once
#pragma GCC system_header

#include <tuple>
#include <string>
#include <type_traits>
#include "define_type_traits.h"
#include "string_literal.h"

#if 0
namespace csrl {

template <class T>
[[gnu::no_sanitize_undefined]]
auto get_field_names() {
  using Type = csrl::remove_cvref_t<T>;
  if constexpr (std::is_pointer_v<Type>) {
    return get_field_names<std::remove_pointer_t<T>>();
  } else {
    const auto get = []<std::size_t... _is>(std::index_sequence<_is...>) {
      return concat_literals(
          get_field_name<Type, get_ith_field_from_fake_object<T, _is>()>()...);
    };
    return get(std::make_index_sequence<num_fields<T>>());
  }
}

}  
#endif