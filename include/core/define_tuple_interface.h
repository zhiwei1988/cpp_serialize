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

#define PP_FOR_EACH_I_5(M, STRUCT_NAME, F0, F1, F2, F3, F4)                                                            \
    PP_FOR_EACH_I_4(M, STRUCT_NAME, F0, F1, F2, F3) M(4, STRUCT_NAME, F4)

#define PP_FOR_EACH_I_6(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5)                                                        \
    PP_FOR_EACH_I_5(M, STRUCT_NAME, F0, F1, F2, F3, F4) M(5, STRUCT_NAME, F5)

#define PP_FOR_EACH_I_7(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6)                                                    \
    PP_FOR_EACH_I_6(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5) M(6, STRUCT_NAME, F6)

#define PP_FOR_EACH_I_8(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7)                                                \
    PP_FOR_EACH_I_7(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6) M(7, STRUCT_NAME, F7)

#define PP_FOR_EACH_I_9(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7, F8)                                            \
    PP_FOR_EACH_I_8(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7) M(8, STRUCT_NAME, F8)

#define PP_FOR_EACH_I_10(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7, F8, F9)                                       \
    PP_FOR_EACH_I_9(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7, F8) M(9, STRUCT_NAME, F9)

#define PP_FOR_EACH_I_11(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10)                                  \
    PP_FOR_EACH_I_10(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7, F8, F9) M(10, STRUCT_NAME, F10)

#define PP_FOR_EACH_I_12(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11)                             \
    PP_FOR_EACH_I_11(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10) M(11, STRUCT_NAME, F11)

#define PP_FOR_EACH_I_13(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12)                        \
    PP_FOR_EACH_I_12(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11) M(12, STRUCT_NAME, F12)

#define PP_FOR_EACH_I_14(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13)                   \
    PP_FOR_EACH_I_13(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12) M(13, STRUCT_NAME, F13)

#define PP_FOR_EACH_I_15(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14)              \
    PP_FOR_EACH_I_14(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13)                       \
        M(14, STRUCT_NAME, F14)

#define PP_FOR_EACH_I_16(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15)         \
    PP_FOR_EACH_I_15(M, STRUCT_NAME, F0, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14)                  \
        M(15, STRUCT_NAME, F15)

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
    template <> \
    inline decltype(auto) get<INDEX>(STRUCT_NAME & s)                                   \
    {                                                                                                                  \
        return (s.FIELD_NAME(FIELD_PAIR));                                                                               \
    }\
    template <> \
    inline decltype(auto) get<INDEX>(const STRUCT_NAME & s)                                   \
    {                                                                                                                  \
        return (s.FIELD_NAME(FIELD_PAIR));                                                                               \
    }

// 辅助宏：用于字符串化字段名
#define STRINGIFY_FIELD_NAME(FIELD_PAIR) STRINGIFY_FIELD_NAME_IMPL(FIELD_NAME(FIELD_PAIR))
#define STRINGIFY_FIELD_NAME_IMPL(NAME) STRINGIFY_FIELD_NAME_IMPL2(NAME) // 保证FIELD_NAME(FIELD_PAIR)可以展开
#define STRINGIFY_FIELD_NAME_IMPL2(NAME) #NAME

namespace csrl {
    template<typename StructType, std::size_t Index>
    struct FieldNameGetter;
}

// 生成字段名获取器的宏
#define GEN_FIELD_NAME_GETTER(INDEX, STRUCT_NAME, FIELD_PAIR)                                                          \
    template <>                                                                                                        \
    struct csrl::FieldNameGetter<STRUCT_NAME, INDEX> {                                                                 \
        static const char* Get() { return STRINGIFY_FIELD_NAME(FIELD_PAIR); }                                          \
    };

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
    } \
    EXPAND_ARGS_AS_FOR_EACH(STRUCT_NAME, GEN_FIELD_NAME_GETTER, __VA_ARGS__)                                                                                                                 
