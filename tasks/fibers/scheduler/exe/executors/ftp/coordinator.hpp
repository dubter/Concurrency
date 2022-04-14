#pragma once

#include <twist/stdlike/atomic.hpp>

namespace exe::executors::ftp {

// Coordinates workers (stealing, parking)

class Coordinator {
 public:
  // ???

 private:
  twist::stdlike::atomic<size_t> active_{0};
  twist::stdlike::atomic<size_t> stealing_{0};

  // ???
};

}  // namespace exe::executors::ftp
