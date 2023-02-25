#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>

#include <cstdint>

// CyclicBarrier allows a set of threads to all wait for each other
// to reach a common barrier point

// The barrier is called cyclic because
// it can be re-used after the waiting threads are released.

class CyclicBarrier {
 public:
  explicit CyclicBarrier(size_t /*participants*/) {
    // Not implemented
  }

  void ArriveAndWait() {
    // Not implemented
  }

 private:
};
