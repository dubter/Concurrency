#include <tinyfibers/sync/parking_lot.hpp>

#include <tinyfibers/core/scheduler.hpp>

namespace tinyfibers {

namespace detail {

void ParkingLot::Park() {
  waitee_ = GetCurrentFiber();
  GetCurrentScheduler()->Suspend();
}

void ParkingLot::Wake() {
  if (waitee_ != nullptr) {
    GetCurrentScheduler()->Resume(waitee_);
  }
}

}  // namespace detail

}  // namespace tinyfibers
