//
// Created by loki7 on 25-6-29.
//


#pragma once

#include <vector>
#include <nyx/common.h>

namespace nyx::ecs::detail
{
    struct system
    {
        std::vector<size_type> read_component_ids;
        std::vector<size_type> write_component_ids;
    };
}
