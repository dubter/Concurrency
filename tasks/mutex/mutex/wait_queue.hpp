#pragma once

#include <twist/stdlike/atomic.hpp>

namespace solutions {

class WaitQueue {
 public:
  void Park() {
    queue_.wait(0);
  }

  void WakeOne() {
    queue_.notify_one();
  }

  void WakeAll() {
    queue_.notify_all();
  }

 private:
  twist::stdlike::atomic<uint32_t> queue_{0};
};

}  // namespace solutions
