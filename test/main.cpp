#include <iostream>
#include "nyx/ecs.hpp"


struct vector_2d
{
    int x;
    int y;
    std::vector<int> v;
};

int main()
{
    nyx::ecs::sparse_set<vector_2d> storage;
    storage.set(10, vector_2d{100, 200, {1, 2, 3, 4}});
    storage.set(20, vector_2d{1, 2, {7,7,7,7}});

    auto t = storage.get(10);

    storage.remove(10);
    storage.remove(20);

    storage.shrink_to_fit();

    auto map = nyx::ecs::dense_map<std::string, vector_2d>();
    map.set("Hello World", vector_2d{1, 2});

    auto value = map.get("Hello World");

    return 0;
}
