#pragma once

#include <twist/stdlike/atomic.hpp>
#include <twist/util/spin_wait.hpp>

namespace spinlocks {

/*  Scalable Queue SpinLock
 *
 *  Usage:
 *  {
 *    QueueSpinLock::Guard guard(spinlock);  // <-- Lock
 *    // Critical section
 *  }  // <-- Unlock
 */

class QueueSpinLock {
 public:
  class Guard {
   public:
    explicit Guard(QueueSpinLock& spinlock) : spinlock_(spinlock) {
      // Your code goes here
    }

    ~Guard() {
      // Your code goes here
    }

   private:
    QueueSpinLock& spinlock_;
    // ???
  };

 private:
  // ???
};

}  // namespace spinlocks
