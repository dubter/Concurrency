#pragma once

#include <exe/fibers/core/api.hpp>

#include <twist/ed/stdlike/atomic.hpp>

#include <wheels/intrusive/list.hpp>
#include <wheels/core/assert.hpp>

namespace exe::fibers {

template <typename T>
class FutexLike {
 public:
  explicit FutexLike(twist::ed::stdlike::atomic<T>& /*cell*/) {
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
