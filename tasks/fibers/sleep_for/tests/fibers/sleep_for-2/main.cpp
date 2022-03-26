#include <wheels/test/test_framework.hpp>
#include <wheels/test/util.hpp>

#include <wheels/support/cpu_time.hpp>

#include <exe/fibers/core/api.hpp>

#include "../common/run.hpp"

using namespace exe;
using namespace std::chrono_literals;

//////////////////////////////////////////////////////////////////////

TEST_SUITE(SleepFor) {
  SIMPLE_TEST(JustWorks) {
    RunScheduler(/*threads=*/4, []() {
      for (size_t i = 0; i < 17; ++i) {
        fibers::self::SleepFor(100ms);
        std::cout << i << std::endl;
      }
    });
  }

  void StressTest(size_t fibers, size_t sleeps) {
    RunScheduler(/*threads=*/4, [fibers, sleeps]() {
      for (size_t i = 0; i < fibers; ++i) {
        fibers::Go([i, sleeps]() {
          for (size_t j = 0; j < sleeps; ++j) {
            fibers::self::SleepFor(((i + j) % 7) * 1ms);

            if (j % 11 == 0) {
              fibers::self::Yield();
            }
          }
        });
      }
    });
  }

  SIMPLE_TEST(Stress1) {
    StressTest(/*fibers=*/5, /*sleeps=*/1024);
  }

  SIMPLE_TEST(Stress2) {
    StressTest(/*fibers=*/2, /*sleeps=*/1024);
  }

  SIMPLE_TEST(Stress3) {
    StressTest(/*fibers=*/10, /*sleeps=*/512);
  }

  void StressTestSmallSleeps(size_t fibers) {
    while (wheels::test::KeepRunning()) {
      RunScheduler(/*threads=*/4, [fibers]() {
        for (size_t i = 0; i < fibers; ++i) {
          fibers::Go([i] {
            fibers::self::SleepFor((i % 2) * 1ms);
          });
        }
      });
    }
  }

  SIMPLE_TEST(StressSmallSleep1) {
    StressTestSmallSleeps(/*fibers=*/1);
  }

  SIMPLE_TEST(StressSmallSleep2) {
    StressTestSmallSleeps(/*fibers=*/2);
  }
}

RUN_ALL_TESTS()
