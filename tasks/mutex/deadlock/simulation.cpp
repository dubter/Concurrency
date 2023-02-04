#include <wheels/test/test_framework.hpp>

// https://gitlab.com/Lipovsky/tinyfibers

#include <tinyfibers/sched/spawn.hpp>
#include <tinyfibers/sched/yield.hpp>
#include <tinyfibers/sync/mutex.hpp>
#include <tinyfibers/sync/wait_group.hpp>

#include <tinyfibers/rt/scheduler.hpp>

#include <wheels/core/panic.hpp>

#include <wheels/system/quick_exit.hpp>

using tinyfibers::Mutex;
using tinyfibers::Spawn;
using tinyfibers::WaitGroup;
using tinyfibers::self::Yield;

using tinyfibers::rt::Scheduler;

TEST_SUITE(Deadlock) {
  // Deadlock with one fiber and one mutex
  TEST(OneFiber, wheels::test::TestOptions().ForceFork()) {
    Scheduler scheduler;

    scheduler.SetDeadlockHandler([]() {
      std::cout << "Fiber deadlocked!" << std::endl;
      // World is broken, leave it ASAP
      wheels::QuickExit(0);
    });

    scheduler.Run([]() {
      Mutex mutex;

      auto locker = [&]() {
        // Your code goes here
        // use mutex.Lock() / mutex.Unlock() to lock/unlock mutex
      };

      Spawn(locker).Join();

      // We do not expect to reach this line
      WHEELS_PANIC("No deadlock =(");
    });
  }

  // Deadlock with two fibers
  TEST(TwoFibers, wheels::test::TestOptions().ForceFork()) {
    Scheduler scheduler;

    scheduler.SetDeadlockHandler([] {
      std::cout << "Fibers deadlocked!" << std::endl;
      // World is broken, leave it ASAP
      wheels::QuickExit(0);
    });

    scheduler.Run([]() {
      // Mutexes

      Mutex a;
      Mutex b;

      // Fiber routines

      auto first = [&]() {
        // Your code goes here
        // Use Yield() to reschedule current fiber
      };

      auto second = [&]() {
        // Your code goes here
      };

      // No deadlock with one fiber

      // No deadlock expected here
      // Run routine twice to check that
      // routine leaves mutexes in unlocked state
      Spawn(first).Join();
      Spawn(first).Join();

      // Same for `second`
      Spawn(second).Join();
      Spawn(second).Join();

      // Deadlock with two fibers

      WaitGroup wg;
      wg.Spawn(first);
      wg.Spawn(second);
      wg.Wait();

      // We do not expect to reach this line
      WHEELS_PANIC("No deadlock =(");
    });
  }
}

RUN_ALL_TESTS()
