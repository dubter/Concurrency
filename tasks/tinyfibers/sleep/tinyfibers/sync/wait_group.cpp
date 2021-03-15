#include <tinyfibers/sync/wait_group.hpp>

#include <wheels/support/assert.hpp>

namespace tinyfibers {

WaitGroup& WaitGroup::Spawn(FiberRoutine routine) {
  join_handles_.push_back(::tinyfibers::Spawn(std::move(routine)));
  return *this;
}

void WaitGroup::Wait() {
  for (auto& h : join_handles_) {
    h.Join();
  }
  join_handles_.clear();
}

WaitGroup::~WaitGroup() {
  WHEELS_VERIFY(join_handles_.empty(), "Explicit Wait required");
}

}  // namespace tinyfibers
