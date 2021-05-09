#pragma once

#include <wheels/support/result.hpp>

#include <experimental/coroutine>

#include <optional>

namespace task {

template <typename T>
class [[nodiscard]] Task {
 public:
  struct Promise;

  using AnyCoroHandle = std::experimental::coroutine_handle<>;
  using MyCoroHandle = std::experimental::coroutine_handle<Promise>;

  struct Promise {
    auto get_return_object() {
      return Task{};
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

    void return_value(T /*value*/) {
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

    T await_resume() {
      std::abort();  // Not implemented
    }
  };

  Task() {
    // Not implemented
  }

  Task(Task&&) {
    // Not implemented
  }

  Task& operator=(Task&&) {
    return *this;  // Not implemented
  }

  // Non-copyable
  Task(const Task&) = delete;
  Task& operator=(const Task&) = delete;

  ~Task() {
    // Not implemented
  }

  auto operator co_await() {
    return Awaiter{};
  }

 private:
  MyCoroHandle coro_;
};

}  // namespace task

template <typename T, typename... Args>
struct std::experimental::coroutine_traits<task::Task<T>, Args...> {
  using promise_type = typename task::Task<T>::Promise;
};
