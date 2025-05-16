#include <nyx/ecs.hpp>
#include <iostream>

#include "nyx/chunk.hpp"


int main()
{
    nyx::ecs::chunk<int> chunk;
    auto size = decltype(chunk)::max_size;
    chunk[0] = 1;
    chunk[1] = 2;
    chunk[2] = 3;

    return 0;
}
