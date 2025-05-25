#include <iostream>
#include "fields.h"
#include <string>
#include <cstddef>
#include <tuple>
#include "define_tuple_interface.h"

struct Test {
    int a;
    int b;
    int c;
};

int main() {
    std::cout << csrl::FieldsCounts<Test> << std::endl;
    return 0;
}
