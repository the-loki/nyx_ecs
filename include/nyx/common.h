//
// Created by loki7 on 25-6-26.
//


#pragma once

#include <string>
#include <string_view>
#include <source_location>

namespace nyx::ecs::detail
{
    using size_type = size_t;
    using id_type = size_type;
    using string = std::string;
    using string_view = std::string_view;
    using source_location = std::source_location;
} // namespace nyx::ecs::detail
