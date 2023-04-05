#pragma once

#include "tagged_semaphore.hpp"

#include <deque>

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue

template <typename T>
class BoundedBlockingQueue {
 public:
  explicit BoundedBlockingQueue(size_t capacity)
      : mutex_(1),
        free_elems_(capacity),
        elems_in_queue_(0) {
  }

  void Put(T value) {
    auto token = free_elems_.Acquire();
    mutex_.Acquire();
    queue_.push_back(std::move(value));
    mutex_.Release();
    elems_in_queue_.Release(std::move(token));
  }

  T Take() {
    auto token = elems_in_queue_.Acquire();
    mutex_.Acquire();
    T head = std::move(queue_.front());
    queue_.pop_front();
    mutex_.Release();
    free_elems_.Release(std::move(token));
    return head;
  }

 private:
  Semaphore mutex_;
  TaggedSemaphore<int> free_elems_;
  TaggedSemaphore<int> elems_in_queue_;
  std::deque<T> queue_;
};
