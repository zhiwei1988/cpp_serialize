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

#if 0
struct Person { std::string name; int age; double height; };

namespace std {
    template <>
    struct tuple_size<::Person> : std::integral_constant<std::size_t, 3> {};

    template <> struct tuple_element<0, ::Person> { using type = std::string; };
    template <> struct tuple_element<1, ::Person> { using type = int; };
    template <> struct tuple_element<2, ::Person> { using type = double; };
} // namespace std

template<std::size_t n>
typename std::tuple_element<n, Person>::type& std::get(Person& obj); // 声明

template <> inline std::string& std::get<0>(Person & obj) { return obj.name; } 
template <> inline int& std::get<1>(Person & obj) { return obj.age; } 
template <> inline double& std::get<2>(Person & obj) { return obj.height; }
#endif

int main() {
    std::cout << csrl::FieldsCounts<Test> << std::endl;
    std::get<0>(p);
    return 0;
}
