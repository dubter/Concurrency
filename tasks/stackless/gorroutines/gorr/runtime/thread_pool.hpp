#pragma once

#include <gorr/runtime/detail/wait_queue.hpp>

#include <experimental/coroutine>

#include <twist/stdlike/atomic.hpp>
#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>
#include <twist/stdlike/thread.hpp>

#include <twist/util/thread_local.hpp>

#include <wheels/support/intrusive_list.hpp>

#include <vector>

namespace gorr {

class StaticThreadPool {
 private:
  using CoroHandle = std::experimental::coroutine_handle<>;

  // Awaiter
  struct ScheduleOp {
    bool await_ready() {
      return true;
    }

    void await_suspend(CoroHandle /*handle*/) {
      // Not implemented
    }

    void await_resume() {
      // Nop
    }

    StaticThreadPool& pool;
  };

 public:
  StaticThreadPool(size_t threads);
  ~StaticThreadPool();

  auto Schedule() {
    return ScheduleOp{*this};
  }

  static StaticThreadPool* Current();

  void Join();

 private:
  // ???
};

}  // namespace gorr
