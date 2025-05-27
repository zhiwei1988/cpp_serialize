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

struct Source {
    int id;
    float value;
    
    MAKE_VISITABLE(id, value)
};
    
// 目标结构体
struct Destination {
    float val;
    int identifier;
    
    MAKE_VISITABLE(val, identifier)
};

int main() {
    auto mappingTuple = csrl::MakeFieldMappingTuple(
        csrl::DefaultFieldMapping<1, 0>(),
        csrl::DefaultFieldMapping<0, 1>()
    );

    Source src {1, 2.0f};
    Destination dst {0, 0};

    // 使用自定义转换器的例子
    auto mappingTuple2 = csrl::MakeFieldMappingTuple(
        csrl::FieldMappingWith<1, 0>([](const float& src, float& dst) {
            dst = src * 2.0f; // 将源值乘以2
        }),
        csrl::FieldMappingWith<0, 1>([](const int& src, int& dst) {
            dst = src + 100; // 将源值加上100
        })
    );

    Source src2 {1, 2.0f};
    Destination dst2 {0, 0};
    csrl::StructConverter<Source, Destination, decltype(mappingTuple2)>::Convert(src2, dst2, mappingTuple2);
    std::cout << "Custom converter result: " << dst2.val << " " << dst2.identifier << std::endl;
    csrl::StructConverter<Source, Destination, decltype(mappingTuple)>::Convert(src, dst, mappingTuple);
    std::cout << dst.val << " " << dst.identifier << std::endl;
    return 0;
}

