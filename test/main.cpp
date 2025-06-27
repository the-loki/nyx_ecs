#include <iostream>
#include <nyx/ecs.hpp>

union vector_2d
{
};


int main()
{
    constexpr auto type_name = nyx::ecs::detail::type_utility::get_type_name<vector_2d>();
    std::cout << type_name << std::endl;

    return 0;
}
