#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>

#include <optional>
#include <deque>

namespace exe::tp {

// Unbounded blocking multi-producers/multi-consumers (MPMC) queue

template <typename T>
class UnboundedBlockingQueue {
 public:
  bool Put(T value) {
    std::lock_guard lock(mutex_);
    if (is_closed_) {
      return false;
    }

    queue_.push_back(std::move(value));
    can_take_.notify_one();
    return true;
  }

  std::optional<T> Take() {
    std::unique_lock lock(mutex_);
    while (!is_closed_ && queue_.empty()) {
      can_take_.wait(lock);
    }
    if (queue_.empty()) {
      return std::nullopt;
    }
    T head = std::move(queue_.front());
    queue_.pop_front();
    return head;
  }

  void Close() {
    std::lock_guard guard(mutex_);
    is_closed_ = true;
    can_take_.notify_all();
  }

 private:
  bool is_closed_{false};
  std::deque<T> queue_;
  twist::ed::stdlike::condition_variable can_take_;
  twist::ed::stdlike::mutex mutex_;
};

}  // namespace exe::tp
