#include <iostream>
#include <nyx/ecs.hpp>


struct vector_2d
{
    int x;
    int y;
};


struct vector_3d
{
    int x;
    int y;
};


int main()
{
    using namespace nyx::ecs;

    registry registry;
    registry.get_type_info<vector_2d>();
    registry.get_type_info<vector_3d>();


    registry.get_matched_arch_types<vector_2d, vector_3d>();


    return 0;
}
