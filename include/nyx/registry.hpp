//
// Created by loki7 on 25-6-26.
//


#pragma once

#include <atomic>
#include <mutex>
#include <nyx/dense_map.hpp>
#include <nyx/type_info.hpp>
#include <nyx/type_utility.hpp>

namespace nyx::ecs::detail
{
    class registry
    {
    public:
        registry() = default;
        ~registry() = default;

        template <typename T>
        const type_info* get_type_info();

    protected:
        std::atomic<size_type> type_index_;
        dense_map<std::string, type_info> type_info_map_;

    private:
        size_type get_type_index();

        template <typename T>
        type_info create_type_info();

        std::mutex register_type_mutex_;
    };


    template <typename T>
    const type_info* registry::get_type_info()
    {
        const auto type_name = string(type_utility::get_type_name<T>());

        if (auto type_info = type_info_map_.get(type_name); type_info != nullptr)
        {
            return type_info;
        }

        {
            std::lock_guard<std::mutex> lock(register_type_mutex_);

            if (auto type_info = type_info_map_.get(type_name); type_info != nullptr)
            {
                return type_info;
            }

            type_info_map_.set(type_name, create_type_info<T>());
        }

        return type_info_map_.get(type_name);
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

    inline size_type registry::get_type_index() { return type_index_++; }

} // namespace nyx::ecs::detail
