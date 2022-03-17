#include <wheels/test/test_framework.hpp>

#include <tinyfibers/test/test.hpp>

#include <tinyfibers/api.hpp>
#include <tinyfibers/sync/nursery.hpp>

#include <wheels/support/cpu_time.hpp>
#include <wheels/support/time.hpp>

using namespace std::chrono_literals;

using namespace tinyfibers;

using tinyfibers::self::SleepFor;
using tinyfibers::self::Yield;

TEST_SUITE(SleepFor) {
  TINY_FIBERS_TEST(JustWorks) {
    wheels::StopWatch stop_watch;

    SleepFor(1s);

    auto elapsed = stop_watch.Elapsed();

    ASSERT_TRUE(elapsed >= 1s);
    ASSERT_TRUE(elapsed < 1s + 100ms);
  }

  TINY_FIBERS_TEST(Concurrent) {
    static const size_t kFibers = 100;

    Nursery nursery;

    for (size_t i = 1; i <= kFibers; ++i) {
      nursery.Spawn([i]() {
        SleepFor(10ms * i);
      });
    }

    wheels::StopWatch stop_watch;
    nursery.Wait();
    ASSERT_TRUE(stop_watch.Elapsed() < 1500ms);
  }

  TINY_FIBERS_TEST(DontBurnCPU) {
    wheels::ThreadCPUTimer cpu_timer;

    Spawn([]() {
      SleepFor(1s);
    }).Join();

    ASSERT_TRUE(cpu_timer.Elapsed() < 100ms);
  }

  TINY_FIBERS_TEST(SleepAndRun) {
    size_t runner_steps = 0;

    auto runner = [&]() {
      wheels::StopWatch stop_watch;
      do {
        ++runner_steps;
        Yield();
      } while (stop_watch.Elapsed() < 1s);
    };

    auto sleeper = [&]() {
      SleepFor(1s);
      ASSERT_TRUE(runner_steps >= 1234);
    };

    Nursery nursery;
    nursery.Spawn(runner);
    nursery.Spawn(sleeper);
    nursery.Wait();
  }

  TINY_FIBERS_TEST(SleepQueuePriority) {
    bool stop_requested = false;

    auto runner = [&]() {
      for (size_t i = 0; i < 1234; ++i) {
        Yield();
      }
      stop_requested = true;
    };

    auto sleeper = [&]() {
      size_t count = 0;
      while (!stop_requested) {
        ++count;
        SleepFor(1ms);
      }
    };

    Nursery nursery;
    for (size_t i = 0; i < 10; ++i) {
      nursery.Spawn(sleeper);
    }
    nursery.Spawn(runner);
    nursery.Wait();
  }

  TINY_FIBERS_TEST(RunQueuePriority) {
    bool stop_requested = false;

    auto runner = [&]() {
      while (!stop_requested) {
        Yield();
      }
    };

    auto sleeper = [&]() {
      SleepFor(2s);
      stop_requested = true;
    };

    Nursery nursery;
    nursery.Spawn(runner).Spawn(sleeper).Wait();
  }

  TINY_FIBERS_TEST(PrematureWorkReset) {
    size_t counter = 0;

    auto increase_counter = [&]() {
      SleepFor(1s);
      ++counter;
    };

    Nursery nursery;
    nursery.Spawn(increase_counter);
    SleepFor(std::chrono::seconds(1));
    nursery.Spawn(increase_counter);

    nursery.Wait();

    ASSERT_EQ(counter, 2);
  }
}

RUN_ALL_TESTS()
