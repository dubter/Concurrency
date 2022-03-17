#include <tinyfibers/sync/nursery.hpp>

#include <tinyfibers/core/scheduler.hpp>

#include <wheels/support/assert.hpp>

namespace tinyfibers {

Nursery& Nursery::Spawn(FiberRoutine routine) {
  Fiber* newbie = GetCurrentScheduler()->Spawn(std::move(routine));
  newbie->SetWatcher(this);
  ++active_;
  return *this;
}

void Nursery::Wait() {
  if (active_ > 0) {
    parking_lot_.Park();
  }
}

void Nursery::OnCompleted() {
  if (--active_ == 0) {
    // Last fiber
    parking_lot_.Wake();
  }
}

Nursery::~Nursery() {
  WHEELS_VERIFY(active_ == 0, "Wait required");
}

}  // namespace tinyfibers
