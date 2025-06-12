#pragma once

#include <cstdint>
#include "string_literal.h"

namespace csrl
{

template <typename T> 
struct LiteralHelper
{
    constexpr static T helper_ = T();
    constexpr static auto name_ = helper_.arr_;
};

template <typename... StringLiterals> 
class Literal
{
    using FieldsType = std::tuple<LiteralHelper<StringLiterals>...>;

  public:
    using ValueType = std::conditional_t<sizeof...(StringLiterals) <= 255, uint8_t, uint16_t>;

    /// The number of different fields or different options that the literal
    /// can assume.
    static constexpr ValueType num_fields_ = sizeof...(StringLiterals);

    using ReflectionType = std::string;

    /// Constructs a Literal from another literal.
    //Literal(const Literal<StringLiterals...>& _other) = default;
};
} // namespace csrl