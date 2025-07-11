//
// Created by loki7 on 25-7-11.
//


#pragma once


namespace nyx::ecs::detail
{
    template <typename T>
    class sparse_set
    {
    public:
        using value_type = T;

        explicit sparse_set(size_type chunk_size);
    private:
        size_type chunk_size_;
    };

    template <typename T>
    sparse_set<T>::sparse_set(size_type chunk_size = chunk_capacity) : chunk_size_(chunk_size)
    {
    }


} // namespace nyx::ecs::detail
