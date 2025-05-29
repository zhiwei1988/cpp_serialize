#pragma once

#include <tuple>
#include <string>
#include <utility> 
#include <cstddef> 

// 连接两个预处理器符号
#define PP_CAT(A, B) PP_CAT_I(A, B)
#define PP_CAT_I(A, B) A##B

// 获取可变参数宏中的参数数量 (这里支持最多16个字段，可以扩展)
// 注意：每个 ((Type, Name)) 对被视为一个参数
#define PP_NARG(...) PP_NARG_I(__VA_ARGS__, PP_RSEQ_N())
#define PP_NARG_I(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define PP_RSEQ_N() 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

// 预处理器迭代宏 (对每个字段应用一个宏)
// M: 要应用的宏 (它应该接受 INDEX, STRUCT_NAME, FIELD_PAIR 作为参数)
// STRUCT_NAME: 结构体名称
// ...: 字段对列表，例如 ((int, x)), ((double, y))
#define PP_FOR_EACH_I_0(M, STRUCT_NAME)
#define PP_FOR_EACH_I_1(M, STRUCT_NAME, F0) M(0, STRUCT_NAME, F0)
#define PP_FOR_EACH_I_2(M, STRUCT_NAME, F0, F1) PP_FOR_EACH_I_1(M, STRUCT_NAME, F0) M(1, STRUCT_NAME, F1)
#define PP_FOR_EACH_I_3(M, STRUCT_NAME, F0, F1, F2) PP_FOR_EACH_I_2(M, STRUCT_NAME, F0, F1) M(2, STRUCT_NAME, F2)
#define PP_FOR_EACH_I_4(M, STRUCT_NAME, F0, F1, F2, F3)                                                                \
    PP_FOR_EACH_I_3(M, STRUCT_NAME, F0, F1, F2) M(3, STRUCT_NAME, F3)

#define EXPAND_ARGS_AS_FOR_EACH(STRUCT_NAME, MACRO_TO_APPLY, ...)                                                      \
    PP_CAT(PP_FOR_EACH_I_, PP_NARG(__VA_ARGS__))(MACRO_TO_APPLY, STRUCT_NAME, __VA_ARGS__)

// 从 ((Type, Name)) 中提取 Type 和 Name
#define FIELD_PAIR_GET_TYPE_IMPL(TYPE, NAME) TYPE
#define FIELD_PAIR_GET_NAME_IMPL(TYPE, NAME) NAME
// (Type, Name) 将作为传递给 FIELD_PAIR_GET_TYPE_IMPL 的参数列表
#define FIELD_TYPE(FIELD_PAIR) FIELD_PAIR_GET_TYPE_IMPL FIELD_PAIR
#define FIELD_NAME(FIELD_PAIR) FIELD_PAIR_GET_NAME_IMPL FIELD_PAIR

// 生成结构体成员声明: int x;
#define GEN_STRUCT_MEMBER(INDEX_UNUSED, STRUCT_NAME_UNUSED, FIELD_PAIR) FIELD_TYPE(FIELD_PAIR) FIELD_NAME(FIELD_PAIR);

// 生成 std::tuple_element 特化: template <> struct std::tuple_element<0, MyStruct> { using type = int; };
#define GEN_TUPLE_ELEMENT_SPEC(INDEX, STRUCT_NAME, FIELD_PAIR)                                                         \
    template <> struct tuple_element<INDEX, STRUCT_NAME>                                                          \
    {                                                                                                                  \
        using type = FIELD_TYPE(FIELD_PAIR);                                                                           \
    };

// 重要：为表达式添加括号，可以保证返回的值是左值引用 
// 生成 get<I> 函数模板特化
#define GEN_GET_FUNCTION_SPEC(INDEX, STRUCT_NAME, FIELD_PAIR)                                                          \
    template <> inline decltype(auto) get<INDEX>(STRUCT_NAME & s)                                   \
    {                                                                                                                  \
        return (s.FIELD_NAME(FIELD_PAIR));                                                                               \
    }

namespace std {
    template<std::size_t n, typename T>
    decltype(auto) get(T& obj);
}

// 定义结构体并为其实现元组接口
#define DEFINE_STRUCT_WITH_TUPLE_INTERFACE(STRUCT_NAME, ...)                                                           \
    struct STRUCT_NAME                                                                                                 \
    {                                                                                                                  \
        EXPAND_ARGS_AS_FOR_EACH(STRUCT_NAME, GEN_STRUCT_MEMBER, __VA_ARGS__)                                           \
    };                                                                                                                 \
    namespace std {                                                                                                    \
        template <> struct tuple_size<STRUCT_NAME> : public std::integral_constant<std::size_t, PP_NARG(__VA_ARGS__)> {};\
        EXPAND_ARGS_AS_FOR_EACH(STRUCT_NAME, GEN_TUPLE_ELEMENT_SPEC, __VA_ARGS__)                                          \
        EXPAND_ARGS_AS_FOR_EACH(STRUCT_NAME, GEN_GET_FUNCTION_SPEC, __VA_ARGS__) \
    }                                                                                                                  
