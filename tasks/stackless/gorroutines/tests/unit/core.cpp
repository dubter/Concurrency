#include <gorr/runtime/thread_pool.hpp>
#include <gorr/runtime/yield.hpp>
#include <gorr/runtime/join.hpp>

#include <wheels/test/test_framework.hpp>

#include <twist/stdlike/atomic.hpp>

TEST_SUITE(Core) {
  SIMPLE_TEST(JustWorks) {
    gorr::StaticThreadPool scheduler{4};

    std::atomic<bool> done{false};

    // Do not mix lambdas and coroutines =(
    auto gorroutine = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();

      // Running in thread pool
      ASSERT_EQ(gorr::StaticThreadPool::Current(), &scheduler);

      done.store(true);

      co_return;
    };

    gorr::Detach(gorroutine());  // Spawn

    scheduler.Join();

    ASSERT_TRUE(done.load());
  }

  SIMPLE_TEST(Yield) {
    gorr::StaticThreadPool scheduler{1};

    int step = 0;

    auto gorr1 = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();
      ASSERT_EQ(step++, 0);
      co_await gorr::Yield();
      ASSERT_EQ(step++, 2);
      co_await gorr::Yield();
      ASSERT_EQ(step++, 4);
      co_return;
    };

    auto gorr2 = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();
      ASSERT_EQ(step++, 1);
      co_await gorr::Yield();
      ASSERT_EQ(step++, 3);
      co_await gorr::Yield();
      ASSERT_EQ(step++, 5);
      co_return;
    };

    auto starter = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();

      gorr::Detach(gorr1());
      gorr::Detach(gorr2());
    };

    gorr::Detach(starter());

    scheduler.Join();

    ASSERT_EQ(step, 6);
  }

  class ForkTester {
   public:
    ForkTester(size_t threads) : scheduler_(threads) {
    }

    size_t Explode(size_t d) {
      gorr::Detach(Gorroutine(d));
      scheduler_.Join();
      return leafs_.load();
    }

   private:
    gorr::JoinHandle Gorroutine(int d) {
      co_await scheduler_.Schedule();

      if (d > 2) {
        // Fork
        gorr::Detach(Gorroutine(d - 1));
        gorr::Detach(Gorroutine(d - 2));
      } else {
        ++leafs_;
      }
    }

   private:
    gorr::StaticThreadPool scheduler_;
    std::atomic<size_t> leafs_{0};
  };

  TEST(Forks, wheels::test::TestOptions().AdaptTLToSanitizer()) {
    ForkTester tester{/*threads=*/4};
    ASSERT_EQ(tester.Explode(20), 6765);
  }

  TEST(RacyCounter, wheels::test::TestOptions().AdaptTLToSanitizer()) {
    gorr::StaticThreadPool scheduler{4};

    static const size_t kIncrements = 10'000;
    static const size_t kGorroutines = 256;

    twist::stdlike::atomic<size_t> shared_counter{0};
    twist::stdlike::atomic<size_t> atomic_shared_counter{0};

    auto gorroutine = [&]() -> gorr::JoinHandle {
      co_await scheduler.Schedule();

      for (size_t j = 0; j < kIncrements; ++j) {
        if (j % 7 == 0) {
          co_await gorr::Yield();
        }
        shared_counter.store(shared_counter.load() + 1);
        atomic_shared_counter.fetch_add(1);
      }

      co_return;
    };

    for (size_t i = 0; i < kGorroutines; ++i) {
      gorr::Detach(gorroutine());  // Spawn
    }

    scheduler.Join();

    std::cout << "Shared counter = " << shared_counter.load() << std::endl;
    std::cout << "Atomic shared counter = " << atomic_shared_counter.load()
              << std::endl;

    std::cout << "Expected value = " << kGorroutines * kIncrements << std::endl;

    ASSERT_LT(shared_counter.load(), kGorroutines * kIncrements);
    ASSERT_EQ(atomic_shared_counter.load(), kGorroutines * kIncrements);
  }
};
