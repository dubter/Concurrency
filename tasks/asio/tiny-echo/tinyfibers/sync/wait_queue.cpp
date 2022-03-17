#include <tinyfibers/sync/wait_queue.hpp>

#include <tinyfibers/core/scheduler.hpp>

#include <wheels/support/assert.hpp>

namespace tinyfibers {

namespace detail {

void WaitQueue::Park() {
  Fiber* caller = GetCurrentFiber();
  waiters_.PushBack(caller);
  SuspendCaller();
}

void WaitQueue::WakeOne() {
  if (waiters_.IsEmpty()) {
    return;
  }
  Resume(waiters_.PopFront());
}

void WaitQueue::WakeAll() {
  while (!waiters_.IsEmpty()) {
    Resume(waiters_.PopFront());
  }
}

void WaitQueue::Resume(Fiber* fiber) {
  GetCurrentScheduler()->Resume(fiber);
}

void WaitQueue::SuspendCaller() {
  GetCurrentScheduler()->Suspend();
}

WaitQueue::~WaitQueue() {
  WHEELS_ASSERT(waiters_.IsEmpty(), "WaitQueue is not empty");
}

}  // namespace detail

}  // namespace tinyfibers
