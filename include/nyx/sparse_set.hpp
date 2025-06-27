//
// Created by loki7 on 25-6-26.
//


#pragma once


#include <nyx/common.h>
#include <nyx/flex_array.hpp>

namespace nyx::ecs::detail
{
    template <typename T>
    struct sparse_set
    {
        using value_type = T;

        struct packed_type
        {
            value_type value;
            size_type sparse_index;
        };


        sparse_set() = default;
        sparse_set(const packed_type& o) = delete;
        sparse_set& operator=(const packed_type& o) = delete;
        sparse_set(sparse_set&& o) = default;
        sparse_set& operator=(sparse_set&& o) = default;

        T* get(size_type index);
        void set(size_type index, T&& value);
        void remove(size_type index);
        void shrink_to_fit();

    private:
        size_type size_{0};
        flex_array<packed_type> packed_{{}};
        flex_array<size_type> sparse_{invalid_id};
    };

    template <typename T>
    T* sparse_set<T>::get(const size_type index)
    {
        if (sparse_.size() <= index)
        {
            return nullptr;
        }

        return &(packed_[sparse_[index]].value);
    }

    template <typename T>
    void sparse_set<T>::set(const size_type index, T&& value)
    {
        sparse_.ensure(index);
        if (validate_id(sparse_[index]))
        {
            packed_[sparse_[index]].value = std::forward<T>(value);
            return;
        }

        packed_.ensure(size_);
        sparse_[index] = size_;
        packed_[size_] = {std::forward<T>(value), index};
        size_++;
    }

    template <typename T>
    void sparse_set<T>::remove(const size_type index)
    {
        if (sparse_.size() <= index)
        {
            return;
        }

        if (!validate_id(sparse_[index]))
        {
            return;
        }

        const auto tail_index = size_ - 1;
        auto&& [value, move_index] = packed_[tail_index];
        packed_[sparse_[index]] = {std::move(value), index};
        sparse_[move_index] = sparse_[index];

        sparse_[index] = invalid_id;
        packed_[tail_index] = {T{}, invalid_id};
        size_--;
    }

    template <typename T>
    void sparse_set<T>::shrink_to_fit()
    {
        if (size_ == 0)
        {
            packed_.ensure_chunk_size(0);
            sparse_.ensure_chunk_size(0);
        }
        else
        {
            size_type max_index = 0;
            packed_.ensure(size_ - 1);

            for (size_type i = 0; i < size_; i++)
            {
                max_index = std::max(max_index, packed_[i].sparse_index);
            }

            sparse_.ensure(max_index);
        }

        packed_.shrink_to_fit();
        sparse_.shrink_to_fit();
    }
} // namespace nyx::ecs::detail
