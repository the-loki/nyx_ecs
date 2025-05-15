//
// Created by loki7 on 25-5-15.
//

#pragma once

#include <new>

namespace nyx::ecs
{
#ifdef __cpp_lib_hardware_interference_size
    inline constexpr size_t nyx_chunk_size = {(std::hardware_constructive_interference_size * 64)};
#else
    inline constexpr size_t nyx_chunk_size = {(64 * 64)};
#endif
}
