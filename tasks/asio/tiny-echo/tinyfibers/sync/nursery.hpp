#pragma once

#include <tinyfibers/core/api.hpp>
#include <tinyfibers/core/watcher.hpp>

#include <vector>

namespace tinyfibers {

class Nursery : public IFiberWatcher {
 public:
  Nursery& Spawn(FiberRoutine routine);
  void Wait();

  ~Nursery();

 private:
  void OnCompleted() override;

 private:
  size_t active_{0};
  detail::ParkingLot parking_lot_;
};

}  // namespace tinyfibers
