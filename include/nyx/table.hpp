//
// Created by loki7 on 25-6-28.
//


#pragma once


#include <algorithm>
#include <nyx/hash.hpp>
#include <nyx/sparse_set.hpp>

namespace nyx::ecs::detail
{
    struct entity_store
    {
        size_type size;
        std::byte* data;
    };


    struct table_id
    {
        std::vector<size_type> sorted_column_index_list;

        table_id() = default;
        table_id(const table_id&) = default;

        explicit table_id(const std::vector<size_type>& column_index_list)
        {
            sorted_column_index_list = column_index_list;
            std::sort(sorted_column_index_list.begin(), sorted_column_index_list.end());
        }

        static table_id create(const std::vector<size_type>& column_index_list)
        {
            return table_id(column_index_list);
        }
    };


    struct table
    {
        table_id id;
        size_type size;
        size_type entity_size;
        size_type column_size;
        sparse_set<entity_store> store_set{};
        std::vector<size_type> column_index_list{};
    };


    constexpr size_type fnv_hash(const table_id& key)
    {
        size_type hash = fnv_helper<>::offset;

        for (const auto index : key.sorted_column_index_list)
        {
            hash = (hash ^ static_cast<size_type>(index)) * fnv_helper<>::prime;
        }

        return hash;
    }

    inline bool operator==(const table_id& lhs, const table_id& rhs)
    {
        return lhs.sorted_column_index_list == rhs.sorted_column_index_list;
    }
}
