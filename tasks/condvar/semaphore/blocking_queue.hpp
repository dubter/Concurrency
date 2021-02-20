#pragma once

#include "semaphore.hpp"

#include <deque>

namespace solutions {

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue

template <typename T>
class BlockingQueue {
 public:
  explicit BlockingQueue(size_t /*capacity*/) {
  }

  // Inserts the specified element into this queue
  void Put(T /*value*/) {
    // Not implemented
  }

  // Retrieves and removes the head of this queue,
  // waiting if necessary until an element becomes available
  T Take() {
    return T{};  // Not implemented
  }

 private:
  // Buffer
};

}  // namespace solutions
