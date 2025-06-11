cmake_minimum_required(VERSION 3.20)
project(nyx_ecs)

set(CMAKE_CXX_STANDARD 20)

add_library(nyx_ecs INTERFACE)
target_include_directories(nyx_ecs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(test)