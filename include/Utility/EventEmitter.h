#pragma once

#include <string>
#include <bits/stdc++.h>

template <typename E, typename D>
class EventEmitter
{
public:
    EventEmitter();

    void on(E event, std::function<void(D)> listener);
    void emit(E event, D data);

private:
    std::map<E, std::vector<std::function<void(D)>>> listeners;
};

template <typename E, typename D>
EventEmitter<E, D>::EventEmitter()
{
}

template <typename E, typename D>
void EventEmitter<E, D>::on(E event, std::function<void(D)> listener)
{
    if (listeners.count(event) == 0)
    {
        listeners[event] = std::vector<std::function<void(D)>>();
    }

    listeners[event].push_back(listener);
}

template <typename E, typename D>
void EventEmitter<E, D>::emit(E event, D data)
{
    if (listeners.count(event) == 0)
        return;

    for (auto listener : listeners[event])
    {
        listener(data);
    }
}