#pragma once

#include "atomics.hpp"

#include <twist/strand/stdlike.hpp>
#include <twist/strand/spin_wait.hpp>

#include <wheels/support/cpu.hpp>

namespace solutions {

// Naive Test-and-Set (TAS) spinlock

class TASSpinLock {
 public:
  void Lock() {
    while (AtomicExchange(&locked_, 1) != 0) {
      wheels::SpinLockPause();
    }
  }

  bool TryLock() {
    return false;  // Not implemented
  }

  void Unlock() {
    AtomicStore(&locked_, 0);
  }

 private:
  AtomicInt64 locked_ = 0;
};

}  // namespace solutions
