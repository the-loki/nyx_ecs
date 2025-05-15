cmake_minimum_required(VERSION 3.20)
project(nyx_ecs_test)

set(CMAKE_CXX_STANDARD 20)

add_executable(nyx_ecs_test main.cpp)
target_link_libraries(nyx_ecs_test PRIVATE nyx_ecs)
