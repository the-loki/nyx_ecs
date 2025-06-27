//
// Created by loki7 on 25-6-26.
//


#pragma once

#include <nyx/common.h>

namespace nyx::ecs::detail
{

    struct type_utility
    {
        template <typename T>
        consteval static auto get_type_name()
        {
            return get_pretty_type_name();
        }

    protected:
        consteval static auto get_pretty_type_name(const source_location& loc = source_location::current())
        {
#if defined __clang__ || defined __GNUC__
            auto prefix = '=';
            auto suffix = ']';
#elif defined _MSC_VER
            auto prefix = '<';
            auto suffix = '>';
#else
            static_assert(false, "unsupported compiler.")
#endif

            const string_view full_name = loc.function_name();
            auto start = full_name.find_first_not_of(' ', full_name.find_first_of(prefix) + 1);
            auto value = full_name.substr(start, full_name.find_last_of(suffix) - start);

            if (start = value.find_last_of(' '); start != string_view::npos)
            {
                value = value.substr(start + 1, value.length() - (start + 1));
            }

            return value.substr(0, value.length());
        }
    };


} // namespace nyx::ecs::detail
