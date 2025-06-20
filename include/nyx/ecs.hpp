//
// Created by loki7 on 25-5-15.
//


#pragma once

#include <chrono>
#include <optional>
#include <utility>
#include <vector>

namespace nyx::ecs
{

#ifdef __cpp_lib_hardware_interference_size
    inline constexpr size_t nyx_chunk_size = {(std::hardware_constructive_interference_size * 64)};
#else
    inline constexpr size_t nyx_chunk_size = {(64 * 64)};
#endif

    using size_type = size_t;
    using entity_type = size_t;
    inline constexpr size_type bucket_count = 4096;
    inline constexpr size_type max_size_type = std::numeric_limits<size_type>::max();


    constexpr bool validate_id(size_type value) { return value != max_size_type; }

    constexpr void invalidate_id(size_type& value) { value = max_size_type; }

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

    // fnv1a hash
    constexpr size_type hash_string(const std::string_view view)
    {
        size_type hash = fnv_helper<>::offset;
        for (const auto c : view)
        {
            hash = (hash ^ static_cast<size_type>(c)) * fnv_helper<>::prime;
        }

        return hash;
    }

    constexpr size_type make_hash(const std::string_view view) { return hash_string(view); }


    constexpr size_type make_hash(const size_type value) { return value; }

    template <typename T, size_type ChunkSize = nyx_chunk_size>
    struct chunk
    {
        static constexpr size_type max_size = (ChunkSize / sizeof(T));

        explicit chunk(const T& value);
        chunk(const chunk&) = delete;
        chunk& operator=(const chunk&) = delete;
        chunk(chunk&&) = default;
        chunk& operator=(chunk&&) = default;
        ~chunk();

        T& operator[](size_type index);

    private:
        std::unique_ptr<std::byte[]> store_{nullptr};
    };

    template <typename T, size_type ChunkSize>
    T& chunk<T, ChunkSize>::operator[](size_type index)
    {
        const auto data = reinterpret_cast<T*>(store_.get());
        return data[index];
    }

    template <typename T, size_type ChunkSize>
    chunk<T, ChunkSize>::chunk(const T& value)
    {
        store_ = std::make_unique<std::byte[]>(nyx_chunk_size);

        const auto data = reinterpret_cast<T*>(store_.get());
        std::uninitialized_fill(data, data + max_size, value);
    }

    template <typename T, size_type ChunkSize>
    chunk<T, ChunkSize>::~chunk()
    {
        const auto data = reinterpret_cast<T*>(store_.get());

        if (data == nullptr)
        {
            return;
        }

        for (int i = 0; i < max_size; ++i)
        {
            data[i].~T();
        }
    }

    template <typename T>
    struct flex_array
    {
        T& operator[](size_type index);
        size_type ensure(size_type index);
        void shrink_to_fit();
        void ensure_chunk_size(size_type size);
        [[nodiscard]] size_type size() const;

        explicit flex_array(const T& value);
        flex_array(const flex_array&) = delete;
        flex_array& operator=(const flex_array&) = delete;
        flex_array(flex_array&& o) = default;
        flex_array& operator=(flex_array&& o) = default;

    private:
        size_type size_;
        T default_value_;
        std::vector<chunk<T>> chunks_;
    };

    template <typename T>
    T& flex_array<T>::operator[](size_type index)
    {
        auto chunk_index = index / chunk<T>::max_size;
        auto chunk_offset = index % chunk<T>::max_size;
        return chunks_[chunk_index][chunk_offset];
    }

    template <typename T>
    size_type flex_array<T>::ensure(const size_type index)
    {
        const auto chunk_index = index / chunk<T>::max_size;
        const auto target_chunk_size = chunk_index + 1;

        ensure_chunk_size(target_chunk_size);
        return size_;
    }

    template <typename T>
    void flex_array<T>::shrink_to_fit()
    {
        chunks_.shrink_to_fit();
    }

    template <typename T>
    void flex_array<T>::ensure_chunk_size(size_type size)
    {
        for (auto i = chunks_.size(); i < size; ++i)
        {
            chunks_.emplace_back(default_value_);
        }

        for (auto i = chunks_.size(); i > size; --i)
        {
            chunks_.pop_back();
        }

        size_ = chunks_.size() * chunk<T>::max_size;
    }

    template <typename T>
    size_type flex_array<T>::size() const
    {
        return size_;
    }

    template <typename T>
    flex_array<T>::flex_array(const T& value) : size_(0)
    {
        default_value_ = value;
    }

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
        flex_array<size_type> sparse_{max_size_type};
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
        if (sparse_[index] != max_size_type)
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

        if (sparse_[index] == max_size_type)
        {
            return;
        }

        const auto tail_index = size_ - 1;
        auto&& [value, move_index] = packed_[tail_index];
        packed_[sparse_[index]] = {std::move(value), index};
        sparse_[move_index] = sparse_[index];

        sparse_[index] = max_size_type;
        packed_[tail_index] = {T{}, max_size_type};
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

    template <typename KeyType, typename ValueType, size_type BucketCount = bucket_count>
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
        void remove(const key_type& key);
        value_type* get(const key_type& key);

    private:
        size_type size_{0};
        flex_array<packed_type> packed_{{}};
        flex_array<size_type> sparse_{max_size_type};

        std::optional<std::tuple<size_type, size_type>> find(const key_type& key);
    };


    template <typename KeyType, typename ValueType, size_type BucketCount>
    std::optional<std::tuple<size_type, size_type>>
    dense_map<KeyType, ValueType, BucketCount>::find(const key_type& key)
    {
        const auto index = make_hash(key) % BucketCount;

        if (!validate_id(sparse_[index]))
        {
            return std::nullopt;
        }

        size_type prev_index;
        invalidate_id(prev_index);
        for (size_type curr_index = sparse_[index]; validate_id(curr_index);)
        {
            const auto& packed = packed_[curr_index];

            if (packed.key == key)
            {
                return std::make_tuple(prev_index, curr_index);
            }

            prev_index = curr_index;
            curr_index = packed.next;
        }

        return std::nullopt;
    }

    template <typename KeyType, typename ValueType, size_type BucketCount>
    void dense_map<KeyType, ValueType, BucketCount>::set(const key_type& key, value_type&& value)
    {
        const auto index = make_hash(key) % BucketCount;
        sparse_.ensure(index);
        packed_.ensure(size_);

        auto& packed = packed_[size_];
        packed.key = key;
        packed.sparse_index = index;
        invalidate_id(packed.next);
        packed.value = std::move(value);

        if (!validate_id(sparse_[index]))
        {
            sparse_[index] = size_;
        }
        else
        {
            auto curr = &packed_[sparse_[index]];
            while (validate_id(curr->next))
            {
                curr = &(packed_[curr->next]);
            }

            curr->next = size_;
        }

        size_++;
    }

    template <typename KeyType, typename ValueType, size_type BucketCount>
    void dense_map<KeyType, ValueType, BucketCount>::set(const key_type& key, const value_type& value)
    {
        set(key, std::move(value));
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
            auto prev = prev_index != max_size_type ? &packed_[prev_index] : nullptr;

            if (prev != nullptr)
            {
                prev->next = curr->next;
            }
            else if (validate_id(curr->next))
            {
                sparse_[curr->sparse_index] = curr->next;
            }
            else
            {
                invalidate_id(sparse_[curr->sparse_index]);
            }

            size_--;

            if (size_ == 0)
            {
                return;
            }

            auto [last_prev_index, last_curr_index] = find(packed_[size_].key).value();
            auto last_curr = &packed_[last_curr_index];
            auto last_prev = last_prev_index != max_size_type ? &packed_[last_prev_index] : nullptr;

            if (last_prev == nullptr)
            {
                sparse_[last_curr->sparse_index] = curr_index;
            }
            else
            {
                last_prev->next = curr_index;
            }

            packed_[curr_index] = std::move(*last_curr);
        }
    }

    template <typename KeyType, typename ValueType, size_type BucketCount>
    ValueType* dense_map<KeyType, ValueType, BucketCount>::get(const key_type& key)
    {
        if (auto opt = find(key); opt)
        {
            auto [prev, curr] = opt.value();
            return &packed_[curr].value;
        }

        return nullptr;
    }
} // namespace nyx::ecs
