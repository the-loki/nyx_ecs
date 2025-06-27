#include <iostream>
#include <nyx/ecs.hpp>


namespace XX
{

    struct  vector_2d
    {
        int x;
        int y;
    };
}


int main()
{
    // constexpr auto type_name = nyx::ecs::detail::type_utility::get_type_name<XX::vector_2d>();
    // std::cout << type_name << std::endl;


    return 0;
}
