#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/allocator.hpp>
#include <twist/ed/local/ptr.hpp>

namespace exe::fibers {

static twist::ed::ThreadLocalPtr<Fiber> current;

Fiber::Fiber(Scheduler& scheduler, Routine routine)
    : scheduler_(&scheduler),
      stack_(AllocateStack()),
      my_coroutine_(std::move(routine)) {
}

void Fiber::Schedule() {
  scheduler_->Submit([this] {
    current = this;
    my_coroutine_.Resume();

    if (my_coroutine_.IsCompleted()) {
      delete this;
    } else {
      Schedule();
    }
  });
}

Fiber::~Fiber() {
  ReleaseStack(std::move(stack_));
}

Fiber* Fiber::Self() {
  return current;
}

Scheduler& Fiber::GetScheduler() {
  return *Fiber::Self()->scheduler_;
}

void Fiber::Yield() {
  my_coroutine_.Suspend();
}

}  // namespace exe::fibers
