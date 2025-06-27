//
// Created by loki7 on 25-6-26.
//


#pragma once

#include <source_location>
#include <string>
#include <string_view>

namespace nyx::ecs::detail
{
    using size_type = size_t;
    using id_type = size_type;
    using string = std::string;
    using string_view = std::string_view;
    using source_location = std::source_location;

    inline constexpr size_type invalid_id = std::numeric_limits<size_type>::max();;
    constexpr bool validate_id(size_type value) { return value != invalid_id; }
} // namespace nyx::ecs::detail
