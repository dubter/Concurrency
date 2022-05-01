#include <wheels/test/test_framework.hpp>
#include <wheels/test/util.hpp>

#include <twist/test/test.hpp>
#include <twist/test/util/plate.hpp>

#include <exe/executors/thread_pool.hpp>

#include <exe/tasks/run/fire.hpp>
#include <exe/tasks/run/teleport.hpp>
#include <exe/tasks/run/yield.hpp>

#include <exe/tasks/sync/mutex.hpp>

#include <atomic>
#include <chrono>

using namespace exe;
using namespace std::chrono_literals;

//////////////////////////////////////////////////////////////////////

void StressTest1(size_t fibers) {
  executors::ThreadPool scheduler{4};

  tasks::Mutex mutex;
  twist::test::util::Plate plate;

  auto contender = [&]() -> tasks::Task<> {
    co_await tasks::TeleportTo(scheduler);

    size_t iter = 0;
    while (wheels::test::KeepRunning()) {
      auto lock = co_await mutex.ScopedLock();

      plate.Access();
      if (++iter % 7 == 0) {
        co_await tasks::Yield(scheduler);
      }
    }
  };

  for (size_t i = 0; i < fibers; ++i) {
    tasks::FireAndForget(contender());
  }

  scheduler.WaitIdle();

  std::cout << "# critical sections: " << plate.AccessCount() << std::endl;
  ASSERT_TRUE(plate.AccessCount() > 100500);

  scheduler.Stop();
}

//////////////////////////////////////////////////////////////////////

void StressTest2(size_t gorroutines) {
  executors::ThreadPool scheduler{4};

  while (wheels::test::KeepRunning()) {
    tasks::Mutex mutex;
    size_t cs = 0;

    auto contender = [&]() -> tasks::Task<> {
      co_await tasks::TeleportTo(scheduler);

      {
        auto lock = co_await mutex.ScopedLock();
        ++cs;
      }
    };

    for (size_t j = 0; j < gorroutines; ++j) {
      tasks::FireAndForget(contender());
    }

    scheduler.WaitIdle();

    ASSERT_EQ(cs, gorroutines);
  }

  scheduler.Stop();
}

//////////////////////////////////////////////////////////////////////

TEST_SUITE(GorrMutex) {
  TWIST_TEST_TL(Stress_1_1, 10s) {
    StressTest1(/*fibers=*/4);
  }

  TWIST_TEST_TL(Stress_1_2, 5s) {
    StressTest1(/*fibers=*/16);
  }

  TWIST_TEST_TL(Stress_1_3, 5s) {
    StressTest1(/*fibers=*/100);
  }

  TWIST_TEST_TL(Stress_2_1, 10s) {
    StressTest2(/*fibers=*/2);
  }

  TWIST_TEST_TL(Stress_2_2, 10s) {
    StressTest2(/*fibers=*/3);
  }
}

RUN_ALL_TESTS()
