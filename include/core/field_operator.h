#pragma once

#include <type_traits>
#include <iostream>
#include "define_type_traits.h"

namespace csrl {
struct PrintVisitor {
    mutable bool first = true;

private:
    template<typename T>
    static auto TestPrintable(int) -> decltype(std::cout << std::declval<T>(), std::true_type{});

    template<typename>
    static std::false_type TestPrintable(...);

    template<typename T>
    static constexpr bool IsPrintable = decltype(TestPrintable<T>(0))::value;

    template<typename FieldType>
    static void PrintField(const FieldType& field, std::true_type)
    {
        std::cout << field;
    }

    template<typename FieldType>
    static void PrintField(const FieldType& field, std::false_type)
    {
        std::cout << "[struct]";
    }

public:
    template <typename FieldType>
    void operator()(const FieldType& field, bool isSubStruct)
    {
        if (!first)
            std::cout << " ";
        first = false;

        PrintField(field, std::integral_constant<bool, IsPrintable<FieldType>>{});
    }
};

// 检测类型是否支持 tuple_size（即是否为结构化类型）
template<typename T>
struct has_tuple_size {
private:
    template<typename U>
    static auto test(int) -> decltype(std::tuple_size<remove_cvref_t<U>>::value, std::true_type{});
    template<typename>
    static std::false_type test(...);
public:
    static const bool value = decltype(test<T>(0))::value;
};

template<typename T, typename Visitor>
typename std::enable_if<has_tuple_size<T>::value, void>::type
VisitFields(T& obj, Visitor&& visitor);

template<typename T, typename Visitor>
void VisitFieldsHelper(T& field, Visitor&& visitor, std::true_type) 
{
    VisitFields(field, std::forward<Visitor>(visitor));
}

template<typename T, typename Visitor>
void VisitFieldsHelper(T& field, Visitor&& visitor, std::false_type) {}

template<typename T, std::size_t I, std::size_t N, typename Visitor>
struct FieldVisitor {
    static void Visit(T& obj, Visitor&& visitor) {
        auto &field = std::get<I>(obj);
        constexpr bool isSubStruct = has_tuple_size<decltype(field)>::value;
        visitor(field, isSubStruct);
        VisitFieldsHelper(field, std::forward<Visitor>(visitor), 
                         std::integral_constant<bool, isSubStruct>{});
        
        FieldVisitor<T, I+1, N, Visitor>::Visit(obj, std::forward<Visitor>(visitor));
    }
};

// 递归终止
template<typename T, std::size_t N, typename Visitor>
struct FieldVisitor<T, N, N, Visitor> {
    static void Visit(T& obj, Visitor&& visitor) {}
};

// 允许对结构体内的各个字段执行相同的某个操作
template<typename T, typename Visitor>
typename std::enable_if<has_tuple_size<T>::value>::type
VisitFields(T& obj, Visitor&& visitor)
{
    constexpr std::size_t size = std::tuple_size<T>::value;
    FieldVisitor<T, 0, size, Visitor>::Visit(obj, std::forward<Visitor>(visitor));
}
}