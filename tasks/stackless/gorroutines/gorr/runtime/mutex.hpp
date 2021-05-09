#pragma once

#include <gorr/support/spinlock.hpp>
#include <gorr/runtime/detail/wait_queue.hpp>

#include <experimental/coroutine>

// std::lock_guard
#include <mutex>

namespace gorr {

class Mutex {
  using CoroHandle = std::experimental::coroutine_handle<>;

 private:
  struct Guard {
    ~Guard() {
      mutex.Unlock();
    }

    Mutex& mutex;
  };

  // Awaiter
  struct LockOp {
    bool await_ready() {  // NOLINT
      return true;  // Not implemented
    }

    bool await_suspend(CoroHandle /*handle*/) {  // NOLINT
      return true;  // Not implemented
    }

    auto await_resume() {  // NOLINT
      return Guard{mutex};
    }

    Mutex& mutex;
  };

 public:
  ~Mutex() {
    // Check that waiters list is empty
  }

  auto Lock() {
    return LockOp{*this};
  }

 private:
  void Unlock() {
    // Not implemented
  }

 private:
  // ???
};

}  // namespace gorr
