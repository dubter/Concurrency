#include <gorr/runtime/thread_pool.hpp>
#include <gorr/runtime/yield.hpp>
#include <gorr/runtime/mutex.hpp>
#include <gorr/runtime/join.hpp>

#include <wheels/test/test_framework.hpp>

#include <wheels/support/cpu_time.hpp>

TEST_SUITE(Mutex) {
  SIMPLE_TEST(JustWorks) {
    gorr::StaticThreadPool scheduler{/*threads=*/4};
    gorr::Mutex mutex;

    auto gorroutine = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();

      {
        auto guard = co_await mutex.Lock();
        // Critical section
      }

      {
        auto guard = co_await mutex.Lock();
        // Critical section
      }

      co_return;
    };

    gorr::Detach(gorroutine());  // Spawn

    scheduler.Join();
  }

  SIMPLE_TEST(NestedCriticalSections) {
    gorr::StaticThreadPool scheduler{/*threads=*/4};
    gorr::Mutex mutex1;
    gorr::Mutex mutex2;
    gorr::Mutex mutex3;

    auto gorroutine = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();

      {
        auto guard1 = co_await mutex1.Lock();
        {
          auto guard2 = co_await mutex2.Lock();
          {
            auto guard3 = co_await mutex3.Lock();
          }
        }
      }

      co_return;
    };

    gorr::Detach(gorroutine());  // Spawn

    scheduler.Join();
  }

  SIMPLE_TEST(DoNotBlockPoolThread) {
    gorr::StaticThreadPool scheduler{/*threads=*/2};

    gorr::Mutex mutex;

    size_t cs_count = 0;

    auto bubble = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();
      {
        auto guard = co_await mutex.Lock();
        std::this_thread::sleep_for(1s);
        ++cs_count;
      }
      co_return;
    };

    gorr::Detach(bubble());  // Spawn

    auto locker = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();
      {
        auto guard = co_await mutex.Lock();
        ++cs_count;
      }
      co_return;
    };

    for (size_t i = 0; i < 3; ++i) {
      gorr::Detach(locker());  // Spawn
    }

    std::this_thread::sleep_for(100ms);

    std::atomic<bool> free{false};

    auto runner = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();
      free.store(true);
    };

    gorr::Detach(runner());  // Spawn

    std::this_thread::sleep_for(100ms);

    ASSERT_TRUE(free.load());

    scheduler.Join();

    ASSERT_EQ(cs_count, 4);
  }

#if !__has_feature(address_sanitizer) && !__has_feature(thread_sanitizer)

  SIMPLE_TEST(BlockGorroutine) {
    wheels::ProcessCPUTimer cpu_timer;

    gorr::StaticThreadPool scheduler{/*threads=*/4};

    gorr::Mutex mutex;

    auto sleeper = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();
      {
        auto guard = co_await mutex.Lock();
        std::this_thread::sleep_for(1s);
      }
    };

    gorr::Detach(sleeper());  // Spawn

    std::this_thread::sleep_for(100ms);

    size_t cs_count{0};

    auto locker = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();

      {
        auto guard = co_await mutex.Lock();
        ++cs_count;
      }
    };

    for (size_t i = 0; i < 10; ++i) {
      gorr::Detach(locker());  // Spawn
    }

    scheduler.Join();

    // Do not waste cpu time in waiting lockers
    ASSERT_TRUE(cpu_timer.Elapsed() < 100ms);
    ASSERT_EQ(cs_count, 10);
  }

  SIMPLE_TEST(StackOverflow) {
    gorr::StaticThreadPool scheduler{/*threads=*/1};

    gorr::Mutex mutex;

    static const size_t kLockers = 100500;

    std::atomic<size_t> lockers{0};

    auto sleeper = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();

      auto guard = co_await mutex.Lock();

      while (lockers.load() < kLockers) {
        co_await gorr::Yield();
      }
    };

    gorr::Detach(sleeper());

    std::this_thread::sleep_for(100ms);

    auto locker = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();

      {
        ++lockers;
        auto guard = co_await mutex.Lock();
      }
    };

    for (size_t i = 0; i < kLockers; ++i) {
      gorr::Detach(locker());
    }

    scheduler.Join();
  }

#endif

};
