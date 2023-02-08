#include <wheels/test/framework.hpp>

#include <tinyfibers/sched/spawn.hpp>
#include <tinyfibers/sched/yield.hpp>
#include <tinyfibers/sync/mutex.hpp>
#include <tinyfibers/sync/wait_group.hpp>

using tinyfibers::Mutex;
using tinyfibers::Spawn;
using tinyfibers::WaitGroup;
using tinyfibers::Yield;

// https://gitlab.com/Lipovsky/tinyfibers

// Deadlock with two fibers
void TwoFibersDeadLock() {
  // Mutexes
  Mutex a;
  Mutex b;

  // Fibers

  auto first = [&] {
    // I am a Fiber
  };

  auto second = [&] {
    // I am a Fiber
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
  FAIL_TEST("No deadlock =(");
}
