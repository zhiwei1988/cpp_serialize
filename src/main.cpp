#include <iostream>
#include "field.h"

struct Test {
    int a;
    int b;
};

int main() {
    std::cout << "Hello, CMake!" << std::endl;
    std::cout << csrl::FieldsCounts<Test> << std::endl;
    return 0;
}
