// https://gitlab.com/Lipovsky/tinyfibers
#include <tinyfibers/sched/spawn.hpp>
#include <tinyfibers/sched/yield.hpp>

#include <tinyfibers/sync/mutex.hpp>
#include <tinyfibers/sync/wait_group.hpp>

#include <wheels/core/panic.hpp>

using tinyfibers::Mutex;
using tinyfibers::Spawn;
using tinyfibers::WaitGroup;
using tinyfibers::self::Yield;

// Deadlock with two fibers
void TwoFibersDeadLock() {
  // Mutexes
  Mutex a;
  Mutex b;

  // Fiber routines

  auto first = [&] {
    // Your code goes here
    // Use Yield() to reschedule current fiber
  };

  auto second = [&] {
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
  wg.Spawn(first).Spawn(second).Wait();

  // We do not expect to reach this line
  WHEELS_PANIC("No deadlock =(");
}
