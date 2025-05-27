
#include <iostream>
#include "nyx/ecs.hpp"


struct vector_2d
{
    int x;
    int y;

    vector_2d() = default;

    vector_2d(const int x, const int y) : x(x), y(y)
    {

    }

    vector_2d(vector_2d&& o) noexcept
    {
        x = o.x;
        y = o.y;

        o.x = 0;
        o.y = 0;
    }


    vector_2d& operator=(vector_2d&& o) noexcept
    {
        x = o.x;
        y = o.y;

        o.x = 0;
        o.y = 0;

        return *this;
    }
};



int main()
{
    nyx::ecs::sparse_set<vector_2d> storage;
    storage.set(10, vector_2d{ 100, 200});
    storage.set(20, vector_2d{ 1, 2});

    storage.remove(10);
    storage.remove(20);

    return 0;
}
