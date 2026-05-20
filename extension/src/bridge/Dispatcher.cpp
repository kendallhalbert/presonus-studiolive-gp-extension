#include "bridge/Dispatcher.h"

#include <utility>

namespace presonus::studiolive::gpext::bridge
{

Dispatcher::Dispatcher(std::size_t capacity) : capacity_(capacity > 0 ? capacity : kDefaultCapacity)
{
}

bool Dispatcher::post(Task task)
{
    std::lock_guard lock(mutex_);
    if (tasks_.size() >= capacity_)
    {
        return false;
    }
    tasks_.push(std::move(task));
    return true;
}

void Dispatcher::drain()
{
    while (true)
    {
        Task task;
        {
            std::lock_guard lock(mutex_);
            if (tasks_.empty())
            {
                return;
            }
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        if (task)
        {
            task();
        }
    }
}

std::size_t Dispatcher::pendingCount() const
{
    std::lock_guard lock(mutex_);
    return tasks_.size();
}

} // namespace presonus::studiolive::gpext::bridge
