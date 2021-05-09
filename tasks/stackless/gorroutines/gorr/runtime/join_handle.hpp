#pragma once

#include <experimental/coroutine>

#include <cstdlib>

namespace gorr {

struct JoinHandle {
  struct Promise {
    auto get_return_object() {  // NOLINT
      return JoinHandle{};
    }

    auto initial_suspend() noexcept {  // NOLINT
      return std::experimental::suspend_never{};
    }

    auto final_suspend() noexcept {  // NOLINT
      return std::experimental::suspend_never{};
    }

    void unhandled_exception() {  // NOLINT
      std::terminate();
    }

    void return_void() {  // NOLINT
      // Nop
    }
  };

  JoinHandle() {
    // Not implemented
  }

  JoinHandle(JoinHandle&&) {
    // Not implemented
  }

  JoinHandle& operator=(JoinHandle&&) {
    // Not implemented
    return *this;
  }

  // Non-copyable
  JoinHandle(const JoinHandle&) = delete;
  JoinHandle& operator=(const JoinHandle&) = delete;

  ~JoinHandle() {
    // Not implemented
  }
};

}  // namespace gorr

template <typename... Args>
struct std::experimental::coroutine_traits<gorr::JoinHandle,
                                           Args...> {  // NOLINT
  using promise_type = gorr::JoinHandle::Promise;  // NOLINT
};
