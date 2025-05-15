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

        size_t size{0};

        T& operator[](size_t index);

        chunk();
        ~chunk();
        chunk(const chunk&) = delete;
        chunk operator=(const chunk&) = delete;
        chunk(chunk&& o) noexcept;
        chunk& operator=(chunk&& o) noexcept;

    private:
        T* data{nullptr};
        uint8_t* store{nullptr};
    };

    template <typename T, size_t ChunkSize>
    T& chunk<T, ChunkSize>::operator[](size_t index)
    {
        return data[index];
    }

    template <typename T, size_t ChunkSize>
    chunk<T, ChunkSize>::chunk()
    {
        store = new uint8_t[ChunkSize];
        data = reinterpret_cast<T*>(store);
    }

    template <typename T, size_t ChunkSize>
    chunk<T, ChunkSize>::~chunk()
    {
        for (int i = 0; i < size; ++i)
        {
            data[i].~T();
        }

        size = 0;
        delete[] store;
    }

    template <typename T, size_t ChunkSize>
    chunk<T, ChunkSize>::chunk(chunk&& o) noexcept
        : size(o.size)
          , data(o.data)
          , store(o.store)
    {
        o.size = 0;
        o.data = nullptr;
        o.store = nullptr;
    }

    template <typename T, size_t ChunkSize>
    chunk<T, ChunkSize>& chunk<T, ChunkSize>::operator=(chunk&& o) noexcept
    {
        if (this != &o)
        {
            for (int i = 0; i < size; ++i)
            {
                data[i].~T();
            }

            delete[] store;

            size = o.size;
            data = o.data;
            store = o.store;

            o.size = 0;
            o.data = nullptr;
            o.store = nullptr;
        }
        return *this;
    }
}
