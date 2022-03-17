#pragma once

#include <wheels/intrusive/list.hpp>

namespace tinyfibers {

class Fiber;

namespace detail {

// ~ Futex for cooperative single-threaded fibers

class WaitQueue {
 public:
  ~WaitQueue();

  void Park();

  // Move one fiber to scheduler run queue
  void WakeOne();

  // Move all fibers to scheduler run queue
  void WakeAll();

 private:
  void Resume(Fiber* fiber);
  void SuspendCaller();

 private:
  wheels::IntrusiveList<Fiber> waiters_;
};

}  // namespace detail

}  // namespace tinyfibers
