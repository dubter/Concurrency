#pragma once

#include <wheels/support/result.hpp>

#include <experimental/coroutine>

#include "task.hpp"

#include <optional>

namespace task {

// Better:
// using JoinHandle = Task<void>;

class [[nodiscard]] JoinHandle {
 public:
  struct Promise;

  using AnyCoroHandle = std::experimental::coroutine_handle<>;
  using MyCoroHandle = std::experimental::coroutine_handle<Promise>;

  struct Promise {
    auto get_return_object() {
      return JoinHandle{};
    }

    auto initial_suspend() noexcept {
      // Not implemented
      return std::experimental::suspend_never{};
    }

    auto final_suspend() noexcept {
      // Not implemented
      return std::experimental::suspend_never{};
    }

    void unhandled_exception() {
      std::abort();  // Not implemented
    }

    void return_void() {
      std::abort();  // Not implemented
    }
  };

  struct Awaiter {
    bool await_ready() {
      return true;  // Not implemented
    }

    void await_suspend(AnyCoroHandle /*caller*/) {
      // Not implemented
    }

    void await_resume() {
      // Nop
    }
  };

  auto operator co_await() {
    return Awaiter{};
  }

 private:
  MyCoroHandle coro_;
};

}  // namespace task

template <typename... Args>
struct std::experimental::coroutine_traits<task::JoinHandle, Args...> {
  using promise_type = task::JoinHandle::Promise;
};
