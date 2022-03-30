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

  void StressTest1(size_t fibers) {
    RunScheduler(/*threads=*/4, [fibers]() {
      for (size_t i = 0; i < fibers; ++i) {
        fibers::Go([i]() {
          size_t j = 0;

          while (wheels::test::KeepRunning()) {
            fibers::self::SleepFor(((i + j) % 5) * 1ms);

            if (j % 11 == 0) {
              fibers::self::Yield();
            }

            ++j;
          }
        });
      }
    });
  }

  SIMPLE_TEST(Stress1_1) {
    StressTest1(/*fibers=*/5);
  }

  SIMPLE_TEST(Stress1_2) {
    StressTest1(/*fibers=*/2);
  }

  SIMPLE_TEST(Stress1_3) {
    StressTest1(/*fibers=*/10);
  }

  void StressTest2(size_t fibers) {
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

  SIMPLE_TEST(Stress_2_1) {
    StressTest2(/*fibers=*/1);
  }

  SIMPLE_TEST(Stress_2_2) {
    StressTest2(/*fibers=*/2);
  }

  void NoYieldTest() {
    asio::io_context scheduler;
    bool done = false;

    exe::fibers::Go(scheduler, [&done]() {
        fibers::self::SleepFor(200ms);
        done = true;
    });

    auto processed_count = scheduler.run();

    ASSERT_TRUE(processed_count < 10);
    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(No_Yield_Test) {
    NoYieldTest();
  }
}

RUN_ALL_TESTS()
