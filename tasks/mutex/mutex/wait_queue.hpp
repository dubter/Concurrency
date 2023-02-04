#pragma once

#include <twist/ed/stdlike/atomic.hpp>

namespace util {

class WaitQueue {
 public:
  void Park() {
    // Direct futex syscall, nonstandard
    queue_.FutexWait(0);
  }

  void WakeOne() {
    queue_.FutexWakeOne();
  }

  void WakeAll() {
    queue_.FutexWakeAll();
  }

 private:
  twist::ed::stdlike::atomic<uint32_t> queue_{0};
};

}  // namespace util
