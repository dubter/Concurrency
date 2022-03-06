#pragma once

#include <exe/coroutine/impl.hpp>

namespace exe::fibers {

// Fiber = Stackful Coroutine + Scheduler (Thread pool)

class Fiber {
 public:
  // ~ System calls
  void Schedule();
  void Yield();

  static Fiber& Self();

 private:
  // Task
  void Step();

 private:
  // ???
};

}  // namespace exe::fibers