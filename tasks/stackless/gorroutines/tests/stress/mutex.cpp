#include <gorr/runtime/thread_pool.hpp>
#include <gorr/runtime/yield.hpp>
#include <gorr/runtime/mutex.hpp>
#include <gorr/runtime/join_handle.hpp>

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
      gorroutine();  // Spawn
    }

    scheduler.Join();

    std::cout << "Critical sections: " << plate.AccessCount() << std::endl;

    ASSERT_TRUE(plate.AccessCount() > 100500);
  }

  TWIST_TEST_TL(Stress1, 5s) {
    MutexStressTest(/*threads=*/2, /*gorrs=*/100);
  }

  TWIST_TEST_TL(Stress2, 5s) {
    MutexStressTest(/*threads=*/4, /*gorrs=*/10);
  }

  TWIST_TEST_TL(Stress3, 5s) {
    MutexStressTest(/*threads=*/5, /*gorrs=*/300);
  }

  void MissedWakeupTest(size_t threads, size_t gorroutines) {
    gorr::StaticThreadPool scheduler{threads};

    gorr::Mutex mutex;

    auto goroutine = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();

      {
        auto guard = mutex.Lock();
        // Critical section
        (void)guard;  // Supress warning
      }  // Unlock

      co_return;
    };

    for (size_t i = 0; i < gorroutines; ++i) {
      goroutine();  // Spawn
    };

    scheduler.Join();
  }

  TWIST_ITERATE_TEST(WakeupStress1, 5s) {
    MissedWakeupTest(/*threads=*/2, /*gorroutines=*/2);
  }

  TWIST_ITERATE_TEST(WakeupStress2, 5s) {
    MissedWakeupTest(/*threads=*/2, /*gorroutines=*/3);
  }
}
