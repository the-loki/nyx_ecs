//
// Created by loki7 on 25-6-26.
//


#pragma once

#include <nyx/common.h>

namespace nyx::ecs::detail
{

    struct type_utility
    {


#if defined __clang__ || defined __GNUC__
        consteval static auto pretty(const string_view full_name)
        {






            return name;
        }
#elif defined _MSC_VER

#endif

        template <typename T>
        consteval static auto get_type_name()
        {
            return pretty([](const source_location loc = source_location::current()) { return loc.function_name(); }());
        }
    };


} // namespace nyx::ecs::detail
