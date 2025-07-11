//
// Created by loki7 on 25-7-11.
//


#pragma once

#include <memory>
#include <vector>

#include "common.h"


namespace nyx::ecs::detail
{

    template <typename T>
    struct flex_array
    {
        using value_type = T;
        explicit flex_array(const T& value);

        T& operator[](size_type index);

    private:
        const T value_;
        std::vector<std::unique_ptr<T[]>> store_;
        const size_type chunk_size_ = chunk_capacity;

        void ensure_index(size_type index);
    };


    template <typename T>
    flex_array<T>::flex_array(const T& value) : value_(value)
    {
    }

    template <typename T>
    T& flex_array<T>::operator[](size_type index)
    {
    }

    template <typename T>
    void flex_array<T>::ensure_index(size_type index)
    {
        const auto chunk_index = index / chunk_size_;

        if (chunk_index >= store_.size())
        {
            return;
        }

        for (auto i = store_.size(); i <= chunk_index; ++i)
        {
            store_.emplace_back(std::make_unique<T[]>(chunk_size_));
            std::fill(store_[i].get(), store_[i].get() + chunk_size_, value_);
        }
    }
} // namespace nyx::ecs::detail
