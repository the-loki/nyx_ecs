//
// Created by loki7 on 25-6-26.
//


#pragma once

#include <memory>
#include <vector>

#include <nyx/common.h>

namespace nyx::ecs::detail
{
    template <typename T, size_type ChunkSize = 1024>
    struct flex_array
    {
        flex_array();

        template <typename... Args>
        explicit flex_array(Args&&... args);

        flex_array(const flex_array&) = delete;
        flex_array& operator=(const flex_array&) = delete;
        flex_array(flex_array&& o) = default;
        flex_array& operator=(flex_array&& o) = default;


        T& operator[](size_type index);
        size_type ensure(size_type index);
        void shrink_to_fit();
        void ensure_chunk_size(size_type size);
        [[nodiscard]] size_type size() const;

    private:
        size_type size_;
        const T default_value_;
        std::vector<std::unique_ptr<T[]>> chunks_{};
    };

    template <typename T, size_type ChunkSize>
    flex_array<T, ChunkSize>::flex_array() : default_value_({}), size_(0)
    {
    }

    template <typename T, size_type ChunkSize>
    template <typename... Args>
    flex_array<T, ChunkSize>::flex_array(Args&&... args) :
        size_(0), default_value_(std::forward<Args>(args)...)
    {
    }

    template <typename T, size_type ChunkSize>
    T& flex_array<T, ChunkSize>::operator[](size_type index)
    {
        auto chunk_index = index / ChunkSize;
        auto chunk_offset = index % ChunkSize;

        return chunks_[chunk_index].get()[chunk_offset];
    }

    template <typename T, size_type ChunkSize>
    size_type flex_array<T, ChunkSize>::ensure(size_type index)
    {
        const auto chunk_index = index / ChunkSize;
        const auto target_chunk_size = chunk_index + 1;

        ensure_chunk_size(target_chunk_size);
        return size_;
    }

    template <typename T, size_type ChunkSize>
    void flex_array<T, ChunkSize>::shrink_to_fit()
    {
        chunks_.shrink_to_fit();
    }

    template <typename T, size_type ChunkSize>
    void flex_array<T, ChunkSize>::ensure_chunk_size(size_type size)
    {
        for (auto i = chunks_.size(); i < size; ++i)
        {
            auto chunk = std::make_unique<T[]>(ChunkSize);
            std::fill(chunk.get(), chunk.get() + ChunkSize, default_value_);
            chunks_.emplace_back(std::move(chunk));
        }

        for (auto i = chunks_.size(); i > size; --i)
        {
            chunks_.pop_back();
        }

        size_ = chunks_.size() * ChunkSize;
    }

    template <typename T, size_type ChunkSize>
    size_type flex_array<T, ChunkSize>::size() const
    {
        return size_;
    }
} // namespace nyx::ecs::detail
