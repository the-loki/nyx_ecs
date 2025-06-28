//
// Created by loki7 on 25-6-26.
//


#pragma once

#include <nyx/common.h>

namespace nyx::ecs::detail
{
    template <typename = size_type>
    struct fnv_helper;

    template <>
    struct fnv_helper<uint32_t>
    {
        static constexpr size_type prime = 16777619;
        static constexpr size_type offset = 2166136261;
    };

    template <>
    struct fnv_helper<uint64_t>
    {
        static constexpr size_type prime = 1099511628211;
        static constexpr size_type offset = 14695981039346656037;
    };

    constexpr size_type fnv_hash(const std::string_view key)
    {

        size_type hash = fnv_helper<>::offset;

        for (const auto c : key)
        {
            hash = (hash ^ static_cast<size_type>(c)) * fnv_helper<>::prime;
        }

        return hash;
    }

    constexpr size_type fnv_hash(const size_type key)
    {
        return key;
    }

} // namespace nyx::ecs::detail
