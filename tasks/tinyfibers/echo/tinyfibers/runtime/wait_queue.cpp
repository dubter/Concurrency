#include <tinyfibers/runtime/wait_queue.hpp>

#include <tinyfibers/runtime/scheduler.hpp>

#include <wheels/support/assert.hpp>

namespace tinyfibers {

static inline void Suspend() {
  GetCurrentScheduler()->Suspend();
}

static inline void Resume(Fiber* fiber) {
  GetCurrentScheduler()->Resume(fiber);
}

void WaitQueue::Park() {
  Fiber* caller = GetCurrentFiber();
  waiters_.PushBack(caller);
  Suspend();
}

void WaitQueue::WakeOne() {
  if (waiters_.IsEmpty()) {
    return;
  }
  Fiber* fiber = waiters_.PopFront();
  Resume(fiber);
}

void WaitQueue::WakeAll() {
  while (!waiters_.IsEmpty()) {
    Fiber* fiber = waiters_.PopFront();
    Resume(fiber);
  }
}

WaitQueue::~WaitQueue() {
  WHEELS_ASSERT(waiters_.IsEmpty(), "WaitQueue is not empty");
}

}  // namespace tinyfibers
