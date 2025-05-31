#include <iostream>
#include "fields.h"
#include <string>
#include <cstddef>
#include <tuple>
#include "define_tuple_interface.h"
#include "field_convert.h"

// struct Source {
//     int id;
//     float value;
    
//     MAKE_VISITABLE(id, value)
// };
    
// // 目标结构体
// struct Destination {
//     float val;
//     int identifier;
    
//     MAKE_VISITABLE(val, identifier)
// };

DEFINE_STRUCT_WITH_TUPLE_INTERFACE(Test, (int, a), (int, b), (int, c))
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(Source, (int, id), (float, value), (Test, test))
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(Destination, (float, val), (int, identifier), (Test, test))


int main() {
    Source src {1, 2.0f, Test{1, 2, 3}};
    Destination dst {0, 0, Test{0, 0, 0}};

    Source src2 {1, 2.0f, Test{1, 2, 3}};
    Destination dst2 {0, 0, Test{0, 0, 0}};

    auto mappingTuple = csrl::MakeMappingRuleTuple(
        csrl::MakeFieldMappingRule<1, 0>(),
        csrl::MakeFieldMappingRule<0, 1>(),
        csrl::MakeStructFieldMappingRule<2, 2>(csrl::MakeMappingRuleTuple(
            csrl::MakeFieldMappingRule<0, 0>(),
            csrl::MakeFieldMappingRule<1, 1>(),
            csrl::MakeFieldMappingRule<2, 2>()
        ))
    );

    // 使用自定义转换器的例子
    auto mappingTuple2 = csrl::MakeMappingRuleTuple(
        csrl::MakeFieldMappingCustomRule<1, 0>([](float& src, float& dst) {
            dst = src * 2.0f;
        }),
        csrl::MakeFieldMappingCustomRule<0, 1>([](int& src, int& dst) {
            dst = src + 100;
        }),
        csrl::MakeStructFieldMappingRule<2, 2>(csrl::MakeMappingRuleTuple(
            csrl::MakeFieldMappingCustomRule<0, 0>([](int& src, int& dst) {
                dst = src + 100;
            }),
            csrl::MakeFieldMappingCustomRule<1, 1>([](int& src, int& dst) {
                dst = src + 100;
            }),
            csrl::MakeFieldMappingCustomRule<2, 2>([](int& src, int& dst) {
                dst = src + 100;
            })
        ))
    );

    csrl::StructFieldsConvert(src2, dst2, mappingTuple2);
    std::cout << "Custom converter result: " << dst2.val << " " << dst2.identifier << " " << dst2.test.a << " " << dst2.test.b << " " << dst2.test.c << std::endl;
    csrl::StructFieldsConvert(src, dst, mappingTuple);
    std::cout << dst.val << " " << dst.identifier << " " << dst.test.a << " " << dst.test.b << " " << dst.test.c << std::endl;
    return 0;
}

