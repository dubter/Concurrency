#include <wheels/test/framework.hpp>

#include "sim.hpp"

#include <tinyfibers/rt/scheduler.hpp>

#include <wheels/system/quick_exit.hpp>

using tinyfibers::rt::Scheduler;

TEST_SUITE(LiveLock) {
  TEST(Sim, wheels::test::TestOptions().ForceFork()) {
    Scheduler scheduler;

    // Limit number of scheduler run loop iterations
    scheduler.Run([] { LiveLock(); }, /*fuel=*/123456);

    // World is broken, leave it ASAP
    wheels::QuickExit(0);
  }
}

RUN_ALL_TESTS()
