#pragma once
#include <vector>
#include <span>

namespace lve
{
    template <typename T>

    class EventQueue
    {
        std::vector<T> events;

    public:
        void push(T e) { events.push_back(std::move(e)); }
        const std::vector<T> &read() const { return events; }
        void clear() { events.clear(); }
    };
}
