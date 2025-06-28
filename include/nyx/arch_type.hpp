//
// Created by loki7 on 25-6-28.
//


#pragma once


#include <nyx/type_info.hpp>

#include "dense_map.hpp"

namespace nyx::ecs::detail
{
    struct entity_store
    {
        size_type size;
        std::byte* data;
    };

    struct arch_type
    {
        size_type size;
        size_type entity_size;
        type_info* type_info_list;
        size_type type_info_count;
        dense_map<size_type, entity_store> column;
    };
}
