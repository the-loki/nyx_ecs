
#include <iostream>
#include "nyx/ecs.hpp"


struct vector_2d
{
    int x;
    int y;
};



int main()
{
    auto entity = 9999999;
    nyx::ecs::sparse_set<vector_2d> storage;
    storage.set(entity, vector_2d{.x = 100, .y = 200});

    storage.remove(entity);




    return 0;
}
