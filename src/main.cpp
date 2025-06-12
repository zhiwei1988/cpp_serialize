#include <iostream>
#include "fields.h"
#include <string>
#include <cstddef>
#include <tuple>
#include "define_tuple_interface.h"
#include "field_convert.h"
#include "field_operator.h"

using CharArray = char[10];

DEFINE_STRUCT_WITH_TUPLE_INTERFACE(Test, (int, a), (int, b), (int, c))
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(Source, (int, id), (float, value), (Test, test), (std::string, name))
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(Destination, (float, val), (int, identifier), (Test, test), (CharArray, name))

using namespace csrl;

int main()
{
    Source src{1, 2.0f, Test{1, 2, 3}, "hello"};
    Destination dst{0, 0, Test{0, 0, 0}, "world"};

    Source src2{1, 2.0f, Test{1, 2, 3}, "hello"};
    Destination dst2{0, 0, Test{0, 0, 0}, "world"};

    auto mappingTuple = MakeMappingRuleTuple(
        MakeFieldMappingRule(MakeFieldPath<1>(), MakeFieldPath<0>()),
        MakeFieldMappingRule(MakeFieldPath<0>(), MakeFieldPath<1>()),
        MakeStructFieldMappingRule(MakeFieldPath<2>(), MakeFieldPath<2>(),
                                   MakeMappingRuleTuple(MakeFieldMappingRule(MakeFieldPath<0>(), MakeFieldPath<0>()),
                                                        MakeFieldMappingRule(MakeFieldPath<1>(), MakeFieldPath<1>()),
                                                        MakeFieldMappingRule(MakeFieldPath<2>(), MakeFieldPath<2>()))),
        MakeFieldMappingCustomRule(MakeFieldPath<3>(), MakeFieldPath<3>(),
                                   csrl::StringToCharArrayConverter<std::string, CharArray>));

#if 0
    // 使用自定义转换器的例子
    auto mappingTuple2 = csrl::MakeMappingRuleTuple(
        MakeFieldMappingCustomRule(MakeFieldPath<1>(), MakeFieldPath<0>(), [](float& src, float& dst) {
            dst = src * 2.0f;
        }),
        MakeFieldMappingCustomRule(MakeFieldPath<0>(), MakeFieldPath<1>(), [](int& src, int& dst) {
            dst = src + 100;
        }),
        MakeStructFieldMappingRule(MakeFieldPath<2>(), MakeFieldPath<2>(), csrl::MakeMappingRuleTuple(
            MakeFieldMappingCustomRule(MakeFieldPath<0>(), MakeFieldPath<0>(), [](int& src, int& dst) {
                dst = src + 100;
            }),
            MakeFieldMappingCustomRule(MakeFieldPath<2>(), MakeFieldPath<2>(), [](int& src, int& dst) {
                dst = src + 100;
            })
        )),
        MakeFieldMappingCustomRule(MakeFieldPath<3>(), MakeFieldPath<3>(), csrl::StringToCharArrayConverter<std::string, CharArray>)
    );

    csrl::StructFieldsConvert(src2, dst2, mappingTuple2);
    std::cout << "Custom converter result: " << dst2.val << " " << dst2.identifier << " " << dst2.test.a << " " << dst2.test.b << " " << dst2.test.c << " " << dst2.name << std::endl;
#endif
    csrl::StructFieldsConvert(src, dst, mappingTuple);
    std::cout << dst.val << " " << dst.identifier << " " << dst.test.a << " " << dst.test.b << " " << " " << dst.name
              << std::endl;
    return 0;
}
