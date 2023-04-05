#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/sys.hpp>

#include <cstdlib>

namespace stdlike {

class Mutex {
 public:
  void Lock() {
    counter_.fetch_add(1);
    while (locked_.exchange(1) == 1) {
      twist::ed::Wait(locked_, 1);
    }
    counter_.fetch_sub(1);
  }

  void Unlock() {
    auto wake_key = twist::ed::PrepareWake(locked_);
    locked_.store(0);
    if (counter_.load() > 0) {
      twist::ed::WakeOne(wake_key);
    }
  }

 private:
  twist::ed::stdlike::atomic<uint32_t> locked_{0};
  twist::ed::stdlike::atomic<uint32_t> counter_{0};
};

}  // namespace stdlike
