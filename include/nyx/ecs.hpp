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

        T& operator[](size_type index);

        chunk();
        ~chunk();
        chunk(const chunk&) = delete;
        chunk operator=(const chunk&) = delete;
        chunk(chunk&& o) noexcept;
        chunk& operator=(chunk&& o) noexcept;

    private:
        size_type size_{0};
        T* data_{nullptr};
        std::unique_ptr<std::byte[]> store_{nullptr};
    };

    template <typename T, size_type ChunkSize>
    T& chunk<T, ChunkSize>::operator[](size_type index)
    {
        return data_[index];
    }

    template <typename T, size_type ChunkSize>
    chunk<T, ChunkSize>::chunk()
    {
        store_ = std::make_unique<std::byte[]>(nyx_chunk_size);
        data_ = reinterpret_cast<T*>(store_.get());
        memset(store_.get(), 0xff, ChunkSize);
    }

    template <typename T, size_type ChunkSize>
    chunk<T, ChunkSize>::~chunk()
    {
        for (int i = 0; i < size_; ++i)
        {
            data_[i].~T();
        }

        size_ = 0;
    }

    template <typename T, size_type ChunkSize>
    chunk<T, ChunkSize>::chunk(chunk&& o) noexcept :
        size_(o.size_)
        , data_(o.data_)
        , store_(std::move(o.store_))
    {
        o.size_ = 0;
        o.data_ = nullptr;
        o.store_.reset();
    }

    template <typename T, size_type ChunkSize>
    chunk<T, ChunkSize>& chunk<T, ChunkSize>::operator=(chunk&& o) noexcept
    {
        if (this != &o)
        {
            size_ = o.size_;
            data_ = o.data_;
            store_ = std::move(o.store_);

            o.size_ = 0;
            o.data_ = nullptr;
            o.store_.reset();
        }
        return *this;
    }

    template <typename T>
    struct flex_array
    {
        T& operator[](size_type index);
        size_type ensure(size_type index);
        void shrink_to_fit();
        void ensure_chunk_size(size_type size);
        [[nodiscard]] size_type size() const;

    private:
        size_type size_{0};
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
            chunks_.emplace_back();
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
    struct sparse_set
    {
        using packed_type = std::pair<size_type, T>;

        std::optional<T&> get(size_type index);
        void set(size_type index, T&& value);
        void remove(size_type index);
        void shrink_to_fit();

    private:
        size_type size_{0};
        flex_array<size_type> sparse_;
        flex_array<packed_type> packed_;
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
            packed_[sparse_[index]] = std::make_pair(index, std::forward<T>(value));
            return;
        }

        packed_.ensure(size_);
        sparse_[index] = size_;
        packed_[size_] = std::make_pair(index, std::forward<T>(value));
        ++size_;
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
        auto&& [move_index, value] = packed_[tail_index];
        packed_[sparse_[index]] = {index, std::move(value)};
        sparse_[move_index] = sparse_[index];

        sparse_[index] = max_size_type;
        packed_[tail_index] = std::make_pair(max_size_type, T{});
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
                max_index = std::max(max_index, packed_[i].first);
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
    };
}
