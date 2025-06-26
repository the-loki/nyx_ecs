//
// Created by loki7 on 25-6-26.
//


#pragma once

#include <source_location>
#include <string>

#include <nyx/common.h>

namespace nyx::ecs::detail
{
    struct type_info
    {
        size_type size;
        std::string name;
        size_type alignment;
    };
} // namespace nyx::ecs::detail
