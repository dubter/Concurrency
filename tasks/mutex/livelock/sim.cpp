#include <wheels/test/framework.hpp>

#include <tinyfibers/sched/spawn.hpp>
#include <tinyfibers/sched/yield.hpp>
#include <tinyfibers/sync/wait_group.hpp>

using tinyfibers::WaitGroup;
using tinyfibers::self::Yield;

void LiveLock() {
  static const size_t kIterations = 100;

  size_t cs_count = 0;

  // TrickyLock state
  size_t thread_count = 0;

  auto contender = [&] {
    // Put Yield-s to produce livelock

    for (size_t i = 0; i < kIterations; ++i) {
      // TrickyLock::Lock
      while (thread_count++ > 0) {
        --thread_count;
      }
      // Spinlock acquired

      {
        // Critical section
        ++cs_count;
        ASSERT_TRUE_M(cs_count < 3, "Too many critical sections");
        // End of critical section
      }

      // TrickyLock::Unlock
      --thread_count;
      // Spinlock released
    }
  };

  // Spawn two fibers
  WaitGroup wg;
  wg.Spawn(contender).Spawn(contender).Wait();
};
