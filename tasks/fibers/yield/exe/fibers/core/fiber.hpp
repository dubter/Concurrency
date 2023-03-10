#pragma once

#include <exe/fibers/core/routine.hpp>
#include <exe/fibers/core/scheduler.hpp>

#include <exe/coro/coroutine.hpp>

namespace exe::fibers {

// Fiber = stackful coroutine + scheduler (thread pool)

class Fiber {
 public:
  // ~ System calls
  void Schedule();

  static Fiber* Self();

  // Task
  void Run();

 private:
  // ???
};

}  // namespace exe::fibers
