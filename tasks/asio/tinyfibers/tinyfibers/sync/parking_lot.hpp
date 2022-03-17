#pragma once

namespace tinyfibers {

class Fiber;

namespace detail {

class ParkingLot {
 public:
  void Park();
  void Wake();

 private:
  Fiber* waitee_{nullptr};
};

}  // namespace detail

}  // namespace tinyfibers
