//
// Created by loki7 on 25-5-15.
//


#pragma once

#include <nyx/common.hpp>
#include <cstdint>

namespace nyx::ecs
{
    template <typename T, size_t ChunkSize = nyx_chunk_size>
    struct chunk
    {
        static constexpr size_t max_size = (ChunkSize / sizeof(T));

        size_t size_{0};

        T& operator[](size_t index);

        chunk();
        ~chunk();
        chunk(const chunk&) = delete;
        chunk operator=(const chunk&) = delete;
        chunk(chunk&& o) noexcept;
        chunk& operator=(chunk&& o) noexcept;

    private:
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
        store_ = std::make_unique<std::byte[]>(ChunkSize);
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
    chunk<T, ChunkSize>::chunk(chunk&& o) noexcept
        : size_(o.size_)
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

            o.size = 0;
            o.data_ = nullptr;
            o.store_.reset();
        }
        return *this;
    }
}
