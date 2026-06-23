#include <iostream>
#include "include/NPJ/Vector.h"

int main()
{
    constexpr npj::Vector<int, 4, npj::VectorSemantic::HSVA> a{1,2,3,4};
    npj::Vector<int, 3> b{1,2,3};
    npj::Vector<int, 3> c{5,6,7};
    npj::Vector<int, 3> d = b.cross(c);

    std::cout << a.h << ", " << a.a << "\n";

    return 0;
}
