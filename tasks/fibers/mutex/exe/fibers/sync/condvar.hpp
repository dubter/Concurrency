#pragma once

#include <exe/fibers/sync/mutex.hpp>
#include <exe/fibers/sync/futex.hpp>

#include <twist/stdlike/atomic.hpp>

// std::unique_lock
#include <mutex>

namespace exe::fibers {

class CondVar {
  using Lock = std::unique_lock<Mutex>;

 public:
  void Wait(Lock& /*lock*/) {
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

}  // namespace exe::fibers
