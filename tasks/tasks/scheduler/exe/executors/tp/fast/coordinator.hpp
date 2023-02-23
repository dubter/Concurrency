#pragma once

#include <twist/ed/stdlike/atomic.hpp>

namespace exe::executors::tp::fast {

// Coordinates workers (stealing, parking)

class Coordinator {
 public:
  // ???

 private:
  twist::ed::stdlike::atomic<size_t> active_{0};
  twist::ed::stdlike::atomic<size_t> stealing_{0};

  // ???
};

}  // namespace exe::executors::tp::fast
