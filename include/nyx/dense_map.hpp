//
// Created by loki7 on 25-6-26.
//


#pragma once

#include <nyx/common.h>
#include <nyx/flex_array.hpp>
#include <optional>

#include "hash.hpp"

namespace nyx::ecs::detail
{
    template <typename KeyType, typename ValueType, size_type BucketCount = 256>
    struct dense_map
    {
        using key_type = KeyType;
        using value_type = ValueType;

        struct packed_type
        {
            key_type key;
            value_type value;
            size_type next_index;
            size_type sparse_index;
        };

        struct find_result_type
        {
            packed_type* prev;
            packed_type* curr;
        };

        void set(const key_type& key, value_type&& value);
        void set(const key_type& key, const value_type& value);
        bool has_key(const key_type& key);
        void remove(const key_type& key);
        value_type* get(const key_type& key);

    private:
        size_type size_{0};
        flex_array<packed_type> packed_{{}};
        flex_array<size_type> sparse_{invalid_id};

        std::optional<std::tuple<size_type, size_type>> find(const key_type& key);
    };

    template <typename KeyType, typename ValueType, size_type BucketCount>
    std::optional<std::tuple<size_type, size_type>>
    dense_map<KeyType, ValueType, BucketCount>::find(const key_type& key)
    {
        const auto index = fnv_hash(key) % BucketCount;

        if (sparse_.size() <= index || !validate_id(sparse_[index]))
        {
            return std::nullopt;
        }

        size_type prev_index = invalid_id;
        for (size_type curr_index = sparse_[index]; validate_id(curr_index);)
        {
            const auto& packed = packed_[curr_index];

            if (packed.key == key)
            {
                return std::make_tuple(prev_index, curr_index);
            }

            prev_index = curr_index;
            curr_index = packed.next_index;
        }

        return std::nullopt;
    }

    template <typename KeyType, typename ValueType, size_type BucketCount>
    void dense_map<KeyType, ValueType, BucketCount>::set(const key_type& key, value_type&& value)
    {
        if (auto opt = find(key); opt)
        {
            auto [_, curr_index] = opt.value();
            packed_[curr_index].value = std::forward<value_type>(value);

            return;
        }


        const auto index = fnv_hash(key) % BucketCount;
        sparse_.ensure(index);
        packed_.ensure(size_);

        auto& packed = packed_[size_];

        packed.key = key;
        packed.sparse_index = index;
        packed.next_index = invalid_id;
        packed.value = std::forward<value_type>(value);

        if (!validate_id(sparse_[index]))
        {
            sparse_[index] = size_;
        }
        else
        {
            auto curr = &packed_[sparse_[index]];
            while (validate_id(curr->next_index))
            {
                curr = &packed_[curr->next_index];
            }

            curr->next_index = size_;
        }

        size_++;
    }

    template <typename KeyType, typename ValueType, size_type BucketCount>
    void dense_map<KeyType, ValueType, BucketCount>::set(const key_type& key, const value_type& value)
    {
        set(key, std::move(value));
    }
    template <typename KeyType, typename ValueType, size_type BucketCount>
    bool dense_map<KeyType, ValueType, BucketCount>::has_key(const key_type& key)
    {
        if (auto opt = find(key); opt)
        {
            return true;
        }

        return false;
    }

    template <typename KeyType, typename ValueType, size_type BucketCount>
    void dense_map<KeyType, ValueType, BucketCount>::remove(const key_type& key)
    {
        if (size_ == 0)
        {
            return;
        }

        if (auto opt = find(key); opt)
        {
            auto [prev_index, curr_index] = opt.value();
            auto curr = &packed_[curr_index];
            auto prev = prev_index != invalid_id ? &packed_[prev_index] : nullptr;

            if (prev != nullptr)
            {
                prev->next_index = curr->next_index;
            }
            else if (validate_id(curr->next_index))
            {
                sparse_[curr->sparse_index] = curr->next_index;
            }
            else
            {
                sparse_[curr->sparse_index] = invalid_id;
            }

            size_--;

            if (size_ == 0)
            {
                return;
            }

            auto [last_prev_index, last_curr_index] = find(packed_[size_].key).value();
            auto last_curr = &packed_[last_curr_index];
            auto last_prev = last_prev_index != invalid_id ? &packed_[last_prev_index] : nullptr;

            if (last_prev == nullptr)
            {
                sparse_[last_curr->sparse_index] = curr_index;
            }
            else
            {
                last_prev->next_index = curr_index;
            }

            packed_[curr_index] = std::move(*last_curr);
        }
    }

    template <typename KeyType, typename ValueType, size_type BucketCount>
    ValueType* dense_map<KeyType, ValueType, BucketCount>::get(const key_type& key)
    {
        if (auto opt = find(key); opt)
        {
            auto [_, curr_index] = opt.value();
            return &packed_[curr_index].value;
        }

        return nullptr;
    }


} // namespace nyx::ecs::detail
