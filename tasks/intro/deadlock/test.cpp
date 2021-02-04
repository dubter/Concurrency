#include <wheels/test/test_framework.hpp>
#include <tinyfibers/test/test.hpp>

#include <tinyfibers/runtime/api.hpp>
#include <tinyfibers/runtime/deadlock.hpp>
#include <tinyfibers/sync/mutex.hpp>
#include <tinyfibers/sync/wait_group.hpp>

#include <wheels/support/quick_exit.hpp>
#include <wheels/support/panic.hpp>

using namespace tinyfibers;

TEST_SUITE(Deadlock) {

// Deadlock with one fiber and one mutex
TINY_FIBERS_TEST(OneFiber) {
  Mutex mutex;

  auto locker = [&]() {
    // Your code goes here
    // use mutex.Lock() / mutex.Unlock() to lock/unlock mutex
  };

  SetDeadlockHandler([]() {
    // World is broken, leave it
    wheels::QuickExit(0);
  });

  Spawn(locker).Join();

  // We do not expect to reach this line
  WHEELS_PANIC("No deadlock!");
}

// Deadlock with two fibers
TINY_FIBERS_TEST(TwoFibers) {
  Mutex a;
  Mutex b;

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
  // routine leaves locks in unlocked state
  Spawn(first).Join();
  Spawn(first).Join();

  Spawn(second).Join();
  Spawn(second).Join();

  // Deadlock with two fibers

  SetDeadlockHandler([]() {
    // World is broken, leave it
    wheels::QuickExit(0);
  });

  WaitGroup wg;
  wg.Spawn(first);
  wg.Spawn(second);
  wg.Wait();

  // We do not expect to reach this line
  WHEELS_PANIC("No deadlock!");
}
}

RUN_ALL_TESTS()
