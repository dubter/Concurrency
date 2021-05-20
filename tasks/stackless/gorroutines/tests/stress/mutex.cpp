#include <gorr/runtime/thread_pool.hpp>
#include <gorr/runtime/yield.hpp>
#include <gorr/runtime/mutex.hpp>
#include <gorr/runtime/join.hpp>

#include <twist/test/test.hpp>
#include <twist/test/util/plate.hpp>

#include <wheels/test/test_framework.hpp>
#include <wheels/test/util.hpp>

TEST_SUITE(Mutex) {
  void MutexStressTest(size_t threads, size_t gorroutines) {
    gorr::StaticThreadPool scheduler{threads};

    gorr::Mutex mutex;
    twist::test::util::Plate plate;  // Guarded by mutex

    auto gorroutine = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();

      while (wheels::test::KeepRunning()) {
        auto guard = co_await mutex.Lock();
        {
          // Critical section
          plate.Access();
        }
      }

      co_return;
    };

    for (size_t i = 0; i < gorroutines; ++i) {
      gorr::Detach(gorroutine());  // Spawn
    }

    scheduler.Join();

    std::cout << "Critical sections: " << plate.AccessCount() << std::endl;

    ASSERT_TRUE(plate.AccessCount() > 100500);
  }

  TWIST_TEST_TL(Stress0, 5s) {
    MutexStressTest(/*threads=*/4, /*gorroutines=*/3);
  }

  TWIST_TEST_TL(Stress1, 5s) {
    MutexStressTest(/*threads=*/2, /*gorroutines=*/100);
  }

  TWIST_TEST_TL(Stress2, 5s) {
    MutexStressTest(/*threads=*/4, /*gorroutines=*/10);
  }

  TWIST_TEST_TL(Stress3, 5s) {
    MutexStressTest(/*threads=*/5, /*gorroutines=*/300);
  }

  void MissedWakeupTest(size_t threads, size_t gorroutines) {
    gorr::StaticThreadPool scheduler{threads};

    gorr::Mutex mutex;
    std::atomic<size_t> cs{0};

    auto gorroutine = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();

      {
        auto guard = co_await mutex.Lock();
        ++cs;
        // Critical section
      }

      co_return;
    };

    for (size_t i = 0; i < gorroutines; ++i) {
      gorr::Detach(gorroutine());  // Spawn
    };

    scheduler.Join();

    ASSERT_EQ(cs.load(), gorroutines);
  }

  TWIST_ITERATE_TEST(WakeupStress1, 5s) {
    MissedWakeupTest(/*threads=*/2, /*gorroutines=*/2);
  }

  TWIST_ITERATE_TEST(WakeupStress2, 5s) {
    MissedWakeupTest(/*threads=*/2, /*gorroutines=*/3);
  }
}
