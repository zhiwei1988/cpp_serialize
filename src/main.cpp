#include <iostream>
#include "fields.h"
#include <string>
#include <cstddef>
#include <tuple>
#include "define_tuple_interface.h"
#include "field_convert.h"

struct Test {
    int a;
    int b;
    int c;
};

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

DEFINE_STRUCT_WITH_TUPLE_INTERFACE(Source, (int, id), (float, value))
DEFINE_STRUCT_WITH_TUPLE_INTERFACE(Destination, (float, val), (int, identifier))


int main() {
    Source src {1, 2.0f};
    Destination dst {0, 0};

    Source src2 {1, 2.0f};
    Destination dst2 {0, 0};

    auto mappingTuple = csrl::MakeFieldMappingTuple(
        csrl::DefaultFieldMapping<1, 0>(),
        csrl::DefaultFieldMapping<0, 1>()
    );

    // 使用自定义转换器的例子
    auto mappingTuple2 = csrl::MakeFieldMappingTuple(
        csrl::FieldMappingWith<1, 0>([](float& src, float& dst) {
            dst = src * 2.0f;
        }),
        csrl::FieldMappingWith<0, 1>([](int& src, int& dst) {
            dst = src + 100;
        })
    );

    csrl::StructFieldsConvert(src2, dst2, mappingTuple2);
    std::cout << "Custom converter result: " << dst2.val << " " << dst2.identifier << std::endl;
    csrl::StructFieldsConvert(src, dst, mappingTuple);
    std::cout << dst.val << " " << dst.identifier << std::endl;
    return 0;
}

