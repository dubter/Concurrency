#include <tinyfibers/core/stack_allocator.hpp>

#include <vector>

namespace tinyfibers {

using context::Stack;

//////////////////////////////////////////////////////////////////////

static const size_t kDefaultStackSizeInPages = 8;

//////////////////////////////////////////////////////////////////////

Stack StackAllocator::Allocate() {
  if (auto stack = TryTakeFromPool()) {
    return std::move(*stack);
  }
  return AllocateNew();
}

void StackAllocator::Release(Stack stack) {
  pool_.push_back(std::move(stack));
}

Stack StackAllocator::AllocateNew() {
  return Stack::AllocatePages(kDefaultStackSizeInPages);
}

std::optional<Stack> StackAllocator::TryTakeFromPool() {
  if (pool_.empty()) {
    return std::nullopt;
  }

  Stack stack = std::move(pool_.back());
  pool_.pop_back();
  return stack;
}

}  // namespace tinyfibers
