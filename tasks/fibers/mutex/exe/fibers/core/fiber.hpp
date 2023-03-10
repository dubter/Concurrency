#pragma once

#include <exe/fibers/core/routine.hpp>
#include <exe/fibers/core/scheduler.hpp>

#include <exe/coro/coroutine.hpp>

namespace exe::fibers {

// Fiber = stackful coroutine + scheduler (executor)

class Fiber {
 public:
  // ~ System calls

  void Suspend(/*???*/);

  void Schedule();
  void Switch();

  static Fiber& Self();

 private:
  // Task
  void Run();

 private:
  // ???
};

}  // namespace exe::fibers
