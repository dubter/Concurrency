#pragma once

#include <sure/stack.hpp>
#include <twist/ed/stdlike/mutex.hpp>
#include <vector>

using sure::Stack;

namespace exe::fibers {

sure::Stack AllocateStack();
void ReleaseStack(sure::Stack stack);

class StackAllocator {
 public:
  Stack Allocate() {
    mutex_.lock();
    if (!stacks_.empty()) {
      Stack free_stack = std::move(stacks_.back());
      stacks_.pop_back();
      mutex_.unlock();
      return free_stack;
    }
    mutex_.unlock();
    return AllocateNewStack();
  }

  void Release(Stack stack) {
    std::lock_guard guard(mutex_);
    stacks_.push_back(std::move(stack));
  }

 private:
  static Stack AllocateNewStack() {
    const size_t amount_bytes = 65536;
    return Stack::AllocateBytes(amount_bytes);
  }

 private:
  twist::ed::stdlike::mutex mutex_;
  std::vector<Stack> stacks_;
  // Pool
};

StackAllocator allocator;

sure::Stack AllocateStack() {
  return allocator.Allocate();
}

void ReleaseStack(sure::Stack stack) {
  allocator.Release(std::move(stack));
}

}  // namespace exe::fibers