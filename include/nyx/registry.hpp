//
// Created by loki7 on 25-6-26.
//


#pragma once

#include<ranges>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <nyx/dense_map.hpp>
#include <nyx/type_info.hpp>
#include <nyx/type_utility.hpp>
#include <nyx/table.hpp>

namespace nyx::ecs::detail
{
    class registry
    {
    public:
        registry() = default;
        ~registry() = default;

        template <typename T>
        const type_info* get_type_info();
        const type_info* get_type_info(size_type index);
        const type_info* get_type_info(std::string_view name);

        template <typename... Args>
        std::vector<table*> get_matched_arch_types();

    protected:
        std::atomic<size_type> type_count_;
        dense_map<table_id, table> table_map_;
        flex_array<type_info> type_info_list_;
        dense_map<std::string, size_type> type_info_index_map_;

    private:
        size_type get_type_index();

        template <typename T>
        type_info create_type_info();

        std::shared_mutex register_type_mutex_;
    };

    template <typename T>
    const type_info* registry::get_type_info()
    {
        const auto name = string(type_utility::get_type_name<T>());

        if (const auto type_info = get_type_info(name); type_info != nullptr)
        {
            return type_info;
        }

        {
            std::lock_guard lock(register_type_mutex_);

            if (auto index = type_info_index_map_.get(name); index != nullptr)
            {
                return &type_info_list_[*index];
            }

            auto type_info = create_type_info<T>();
            auto index = type_info.index;
            type_info_list_.ensure(index);
            type_info_list_[index] = std::move(type_info);
            type_info_index_map_.set(name, index);
        }

        return get_type_info(name);
    }

    template <typename T>
    type_info registry::create_type_info()
    {
        return type_info{.size = sizeof(T),
                         .index = get_type_index(),
                         .name = string(type_utility::get_type_name<T>()),
                         .alignment = alignof(T)
        };
    }

    template <typename... Args>
    std::vector<table*> registry::get_matched_arch_types()
    {
        auto type_ids = std::vector{(get_type_info<Args>()->index)...};

        return {};
    }


    inline const type_info* registry::get_type_info(const std::string_view name)
    {
        std::shared_lock lock(register_type_mutex_);

        if (auto index = type_info_index_map_.get(name); index != nullptr)
        {
            return &type_info_list_[*index];
        }

        return nullptr;
    }


    inline const type_info* registry::get_type_info(size_type index)
    {
        std::shared_lock lock(register_type_mutex_);

        if (index < type_count_)
        {
            return &type_info_list_[index];
        }

        return nullptr;
    }

    inline size_type registry::get_type_index() { return type_count_++; }


} // namespace nyx::ecs::detail
