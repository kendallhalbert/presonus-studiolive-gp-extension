#pragma once

#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>

namespace presonus::studiolive::gpext::bridge
{

/// Cross-thread task queue: IO thread posts, GP thread drains.
/// Phase 1+ will replace the mutex-backed deque with a lock-free SPSC ring.
class Dispatcher
{
  public:
    using Task = std::function<void()>;

    /// Enqueue a task. Returns false if the queue is at capacity.
    bool post(Task task);

    /// Run all pending tasks on the calling (GP) thread.
    void drain();

    std::size_t pendingCount() const;

    static constexpr std::size_t kDefaultCapacity = 256;

    explicit Dispatcher(std::size_t capacity = kDefaultCapacity);

  private:
    const std::size_t capacity_;
    mutable std::mutex mutex_;
    std::queue<Task> tasks_;
};

} // namespace presonus::studiolive::gpext::bridge
