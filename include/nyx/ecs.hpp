//
// Created by loki7 on 25-5-15.
//


#pragma once

#include <chrono>
#include <vector>
#include <utility>
#include <optional>

namespace nyx::ecs
{

#ifdef __cpp_lib_hardware_interference_size
    inline constexpr size_t nyx_chunk_size = {(std::hardware_constructive_interference_size * 64)};
#else
inline constexpr size_t nyx_chunk_size = {(64 * 64)};
#endif


    using size_type = size_t;
    using typeid_type = size_t;
    using entity_type = size_t;
    inline constexpr size_t max_size_type = std::numeric_limits<size_type>::max();

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
        std::fill(data, data + max_size, value);
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
    flex_array<T>::flex_array(const T& value):
        size_(0)
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

        std::optional<T&> get(size_type index);
        void set(size_type index, T&& value);
        void remove(size_type index);
        void shrink_to_fit();

    private:
        size_type size_{0};
        flex_array<packed_type> packed_{{}};
        flex_array<size_type> sparse_{max_size_type};
    };

    template <typename T>
    std::optional<T&> sparse_set<T>::get(const size_type index)
    {
        if (packed_.size_ <= index)
        {
            return std::nullopt;
        }

        return packed_[sparse_[index]];
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

    template <typename KeyType, typename ValueType>
    struct dense_map
    {
        using key_type = KeyType;
        using value_type = ValueType;

        struct packed_type
        {
            key_type key;
            size_type next;
            value_type value;
            size_type sparse_index;
        };

        void set(const key_type& key, value_type&& value);
        void set(const key_type& key, const value_type& value);

    private:
        size_type size_{0};
        flex_array<size_type> sparse_{max_size_type};
        flex_array<packed_type> packed_{{}};
    };


    template <typename KeyType, typename ValueType>
    void dense_map<KeyType, ValueType>::set(const key_type& key, value_type&& value)
    {

    }

    template <typename KeyType, typename ValueType>
    void dense_map<KeyType, ValueType>::set(const key_type& key, const value_type& value)
    {
        set(key, std::move(value));
    }
}
