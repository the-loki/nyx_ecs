#include <iostream>
#include <nyx/ecs.hpp>


struct vector_2d
{
    int x;
    int y;
};


int main()
{
    using namespace nyx::ecs;

    registry registry;
    auto type_info = registry.get_type_info<vector_2d>();


    return 0;
}
