#include <wheels/test/framework.hpp>

#include <tinyfibers/sched/spawn.hpp>
#include <tinyfibers/sched/yield.hpp>
#include <tinyfibers/sync/mutex.hpp>

using tinyfibers::Mutex;
using tinyfibers::Spawn;
using tinyfibers::Yield;

// https://gitlab.com/Lipovsky/tinyfibers

void OneFiberDeadLock() {
  Mutex mutex;

  auto fiber = [&] {
    // I am a Fiber
  };

  Spawn(fiber).Join();

  // We do not expect to reach this line
  FAIL_TEST("No deadlock =(");
}
