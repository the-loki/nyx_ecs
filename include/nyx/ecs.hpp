//
// Created by loki7 on 25-5-15.
//


#pragma once

#include <vector>
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
        void fill(const T& value);

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
    void chunk<T, ChunkSize>::fill(const T& value)
    {
        for (int i = 0; i < max_size; i++)
        {
            data_[i] = value;
        }
    }

    template <typename T, size_t ChunkSize>
    chunk<T, ChunkSize>::chunk()
    {
        store_ = std::make_unique<std::byte[]>(nyx_chunk_size);
        data_ = reinterpret_cast<T*>(store_.get());
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
        T& operator[](size_t index);
        void alloc(size_t size, const T& value = T{});
        [[nodiscard]] size_t size() const;

    private:
        size_t size_{0};
        std::vector<chunk<T>> chunks_;
    };

    template <typename T>
    T& flex_array<T>::operator[](size_t index)
    {
        auto chunk_index = index / chunk<T>::max_size;
        auto chunk_offset = index % chunk<T>::max_size;
        return chunks_[chunk_index][chunk_offset];
    }

    template <typename T>
    void flex_array<T>::alloc(const size_t size, const T& value)
    {
        if (size <= size_)
        {
            return;
        }

        const auto chunk_index = size / chunk<T>::max_size;
        for (auto i = chunks_.size(); i < chunk_index + 1; ++i)
        {
            chunks_.emplace_back();
            chunks_[chunks_.size() - 1].fill(value);
        }

        size_ = chunks_.size() * chunk<T>::max_size;
    }

    template <typename T>
    size_t flex_array<T>::size() const
    {
        return size_;
    }

    template <typename T>
    struct sparse_set
    {
        std::optional<T&> get(size_t index);
        void set(size_t index, T&& value);
        void remove(size_t index);

    private:
        size_t size_{0};
        flex_array<T> packed_;
        flex_array<size_t> sparse_;
        flex_array<size_t> packed_index_;
        size_t last_add_index_{0};
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
            sparse_.alloc(index + 1, max_size_t);
        }

        if (sparse_[index] != max_size_t)
        {
            packed_[sparse_[index]] = value;
            return;
        }

        if (size_ >= packed_.size())
        {
            packed_.alloc(size_ + 1);
            packed_index_.alloc(size_ + 1, max_size_t);
        }

        sparse_[index] = size_;
        packed_[size_] = value;
        packed_index_[size_] = index;
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

        packed_[sparse_[index]] = packed_[size_ - 1];
        sparse_[packed_index_[size_ - 1]] = sparse_[index];
        packed_[size_ - 1] = T{};

        packed_index_[sparse_[index]] = packed_index_[size_ - 1];
        packed_index_[size_ - 1] = max_size_t;
        sparse_[index] = max_size_t;

        size_--;
    }


}
