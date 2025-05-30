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


    using ecs_typeid_t = size_t;
    using ecs_entity_t = size_t;
    inline constexpr size_t max_size_t = std::numeric_limits<size_t>::max();


    template <typename T, size_t ChunkSize = nyx_chunk_size>
    struct chunk
    {
        static constexpr size_t max_size = (ChunkSize / sizeof(T));

        T& operator[](size_t index);

        chunk();
        ~chunk();
        chunk(const chunk&) = delete;
        chunk operator=(const chunk&) = delete;
        chunk(chunk&& o) noexcept;
        chunk& operator=(chunk&& o) noexcept;

    private:
        size_t size_{0};
        T* data_{nullptr};
        std::unique_ptr<std::byte[]> store_{nullptr};
    };

    template <typename T, size_t ChunkSize>
    T& chunk<T, ChunkSize>::operator[](size_t index)
    {
        return data_[index];
    }

    template <typename T, size_t ChunkSize>
    chunk<T, ChunkSize>::chunk()
    {
        store_ = std::make_unique<std::byte[]>(nyx_chunk_size);
        data_ = reinterpret_cast<T*>(store_.get());
        memset(store_.get(), 0xff, ChunkSize);
    }

    template <typename T, size_t ChunkSize>
    chunk<T, ChunkSize>::~chunk()
    {
        for (int i = 0; i < size_; ++i)
        {
            data_[i].~T();
        }

        size_ = 0;
    }

    template <typename T, size_t ChunkSize>
    chunk<T, ChunkSize>::chunk(chunk&& o) noexcept :
        size_(o.size_)
        , data_(o.data_)
        , store_(std::move(o.store_))
    {
        o.size_ = 0;
        o.data_ = nullptr;
        o.store_.reset();
    }

    template <typename T, size_t ChunkSize>
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
        void addChunk();
        T& operator[](size_t index);
        size_t resize(size_t size);
        [[nodiscard]] size_t size() const;

    private:
        size_t size_{0};
        std::vector<chunk<T>> chunks_;
    };

    template <typename T>
    void flex_array<T>::addChunk()
    {
        resize(size_ + 1);
    }

    template <typename T>
    T& flex_array<T>::operator[](size_t index)
    {
        auto chunk_index = index / chunk<T>::max_size;
        auto chunk_offset = index % chunk<T>::max_size;
        return chunks_[chunk_index][chunk_offset];
    }

    template <typename T>
    size_t flex_array<T>::resize(const size_t size)
    {
        const auto chunk_index = size / chunk<T>::max_size;
        const auto target_chunk_size = chunk_index + 1;

        if (target_chunk_size == chunks_.size())
        {
            return size_;
        }

        if (target_chunk_size > chunks_.size())
        {
            for (auto i = chunks_.size(); i < target_chunk_size; ++i)
            {
                chunks_.emplace_back();
            }
        }
        else
        {
            for (auto i = chunks_.size(); i > target_chunk_size; --i)
            {
                chunks_.pop_back();
            }
        }

        size_ = chunks_.size() * chunk<T>::max_size;
        return size_;
    }

    template <typename T>
    size_t flex_array<T>::size() const
    {
        return size_;
    }

    template <typename T>
    struct sparse_set
    {
        using packed_type = std::pair<size_t, T>;

        std::optional<T&> get(size_t index);
        void set(size_t index, T&& value);
        void remove(size_t index);
        void shrink_to_fit();

    private:
        size_t size_{0};
        flex_array<size_t> sparse_;
        flex_array<packed_type> packed_;
    };

    template <typename T>
    std::optional<T&> sparse_set<T>::get(const size_t index)
    {
        if (packed_.size_ <= index)
        {
            return std::nullopt;
        }

        return packed_[sparse_[index]];
    }

    template <typename T>
    void sparse_set<T>::set(const size_t index, T&& value)
    {
        if (sparse_.size() <= index)
        {
            sparse_.addChunk();
        }

        if (sparse_[index] != max_size_t)
        {
            packed_[sparse_[index]] = std::make_pair(index, std::forward<T>(value));
            return;
        }

        if (size_ >= packed_.size())
        {
            packed_.addChunk();
        }

        sparse_[index] = size_;
        packed_[size_] = std::make_pair(index, std::forward<T>(value));
        ++size_;
    }

    template <typename T>
    void sparse_set<T>::remove(const size_t index)
    {
        if (sparse_.size() <= index)
        {
            return;
        }

        if (sparse_[index] == max_size_t)
        {
            return;
        }

        const auto tail_index = size_ - 1;
        auto&& [move_index, value] = packed_[tail_index];
        packed_[sparse_[index]] = {index, std::move(value)};
        sparse_[move_index] = sparse_[index];

        sparse_[index] = max_size_t;
        packed_[tail_index] = std::make_pair(max_size_t, T{});

        size_--;
    }

    template <typename T>
    void sparse_set<T>::shrink_to_fit()
    {
        //not impl.
    }

    template <typename KeyType, typename ValueType>
    struct dense_map
    {
        using key_type = KeyType;
        using value_type = ValueType;
    };
}
