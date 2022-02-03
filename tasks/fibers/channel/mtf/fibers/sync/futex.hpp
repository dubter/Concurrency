#pragma once

#include <mtf/fibers/core/handle.hpp>
#include <mtf/fibers/core/suspend.hpp>

#include <twist/stdlike/atomic.hpp>

#include <wheels/intrusive/list.hpp>

namespace mtf::fibers {

// https://eli.thegreenplace.net/2018/basics-of-futexes/

template <typename T>
class FutexLike {
 public:
  explicit FutexLike(twist::stdlike::atomic<T>& /*cell*/) {
  }

  ~FutexLike() {
    // Not implemented
    // Check that wait queue is empty
  }

  // Park current fiber if value of atomic is equal to `old`
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

}  // namespace mtf::fibers
