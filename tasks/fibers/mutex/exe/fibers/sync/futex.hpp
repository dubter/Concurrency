#pragma once

#include <exe/fibers/core/api.hpp>

#include <twist/stdlike/atomic.hpp>

#include <wheels/intrusive/list.hpp>
#include <wheels/support/assert.hpp>

namespace exe::fibers {

template <typename T>
class FutexLike {
 public:
  explicit FutexLike(twist::stdlike::atomic<T>& /*cell*/) {
  }

  ~FutexLike() {
    // Not implemented
    // Check that wait queue is empty
  }

  // Park current fiber if cell.load() == `old`
  void ParkIfEqual(T /*old*/) {
    // Not implemented
  }

  void WakeOne() {
    // Not implemented
  }

  void WakeAll() {
    // Not implemented
  }

 private:
  // ???
};

}  // namespace exe::fibers
