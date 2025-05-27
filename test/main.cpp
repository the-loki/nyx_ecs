
#include <iostream>
#include "nyx/ecs.hpp"


struct vector_2d
{
    int x;
    int y;
};



int main()
{
    nyx::ecs::sparse_set<vector_2d> storage;
    storage.set(10, vector_2d{.x = 100, .y = 200});
    storage.set(20, vector_2d{.x = 1, .y = 2});

    storage.remove(10);
    storage.remove(20);

    return 0;
}
