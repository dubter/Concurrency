#pragma once

#include <wheels/intrusive/list.hpp>

namespace tinyfibers {

// ~ Futex for cooperative _single-threaded_ fibers

// Forward declaration
class Fiber;

class WaitQueue {
 public:
  ~WaitQueue();

  void Park();

  // Move one fiber to scheduler run queue
  void WakeOne();

  // Move all fibers to scheduler run queue
  void WakeAll();

 private:
  wheels::IntrusiveList<Fiber> waiters_;
};

}  // namespace tinyfibers
