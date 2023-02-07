#include <tinyfibers/sched/spawn.hpp>
#include <tinyfibers/sched/yield.hpp>
#include <tinyfibers/sync/mutex.hpp>

#include <wheels/core/panic.hpp>

using tinyfibers::Mutex;
using tinyfibers::Spawn;
using tinyfibers::Yield;

// https://gitlab.com/Lipovsky/tinyfibers

void OneFiberDeadLock() {
  Mutex mutex;

  auto fiber = [&] {
    // Your code goes here
    // use mutex.Lock() / mutex.Unlock() to lock/unlock mutex
  };

  Spawn(fiber).Join();

  // We do not expect to reach this line
  WHEELS_PANIC("No deadlock =(");
}
