
#include "mem_size.hpp"
#include <iostream>

using memory_size::heap_size_of;
using memory_size::total_size_of;

bool vec_test()
{
    bool all_ok = true;
    auto vec1 = std::vector<int>(5, 3);
    if (heap_size_of(vec1) != 20)
    {
        std::cout << "Heap size of vec1 is " << heap_size_of(vec1) << " (20 expected)\n";
        all_ok = false;
    };
    vec1.reserve(100);
    if (heap_size_of(vec1) != 400)
    {
        std::cout << "Heap size of vec1 is " << heap_size_of(vec1) << " (400 expected)\n";
        all_ok = false;
    };
    if (total_size_of(vec1) != 424)
    {
        std::cout << "Total size of vec1 is " << total_size_of(vec1) << " (424 expected)\n";
        all_ok = false;
    };
    return all_ok;
}

int main()
{
    return vec_test() == true ? 0 : 1;
}