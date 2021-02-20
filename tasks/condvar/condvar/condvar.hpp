#pragma once

#include <twist/stdlike/atomic.hpp>

#include <cstdint>

namespace solutions {

class ConditionVariable {
 public:
  // Mutex - BasicLockable
  // https://en.cppreference.com/w/cpp/named_req/BasicLockable
  template <class Mutex>
  void Wait(Mutex& /*mutex*/) {
    // Not implemented
  }

  void NotifyOne() {
    // Not implemented
  }

  void NotifyAll() {
    // Not implemented
  }

 private:
  // ???
};

}  // namespace solutions
