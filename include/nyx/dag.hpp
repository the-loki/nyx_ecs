//
// Created by loki7 on 25-6-29.
//


#pragma once


#include <list>
#include <queue>
#include <vector>
#include <optional>
#include <nyx/common.h>

namespace nyx::ecs::detail
{

    class dag
    {
    public:
        explicit dag(size_type vertex_count);

        void add(size_type from, size_type to);

        std::optional<std::vector<size_type>> topological_sort();

    private:
        size_type vertex_count_{0};
        std::vector<size_type> in_degree_{};
        std::vector<std::list<size_type>> adjacency_list_{};
    };

    inline dag::dag(size_type vertex_count):
        vertex_count_(vertex_count), in_degree_(vertex_count, 0), adjacency_list_(vertex_count)
    {

    }

    inline void dag::add(size_type from, size_type to)
    {
        adjacency_list_[from].push_back(to);
        in_degree_[to]++;
    }

    inline std::optional<std::vector<size_type>> dag::topological_sort()
    {
        std::queue<size_type> queue;
        std::vector<size_type> sequence;

        for (size_type i = 0; i < vertex_count_; i++)
        {
            if (in_degree_[i] == 0)
            {
                queue.push(i);
            }
        }

        size_type count = 0;

        while (!queue.empty())
        {
            size_type current = queue.front();
            queue.pop();
            count++;

            sequence.push_back(current);
            for (auto& to : adjacency_list_[current])
            {
                in_degree_[to]--;

                if (in_degree_[to] == 0)
                {
                    queue.push(to);
                }
            }
        }

        if (count != vertex_count_)
        {
            return std::nullopt;
        }

        return std::move(sequence);
    }


}
