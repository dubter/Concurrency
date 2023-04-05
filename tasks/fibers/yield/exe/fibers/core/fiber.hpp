#pragma once

#include <exe/fibers/core/routine.hpp>
#include <exe/fibers/core/scheduler.hpp>

#include <exe/coro/core.hpp>

namespace exe::fibers {

// Fiber = stackful coroutine + scheduler (thread pool)

class Fiber {
 public:
  void Schedule();

  // Task
  static Fiber* Self();
  static Scheduler& GetScheduler();
  Fiber(Scheduler& scheduler, Routine routine);
  void Yield();
  ~Fiber();

 private:
  Scheduler* scheduler_;
  sure::Stack stack_;
  Coroutine my_coroutine_;
};

}  // namespace exe::fibers
