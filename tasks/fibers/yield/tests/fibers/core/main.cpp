#include <exe/tp/thread_pool.hpp>

#include <exe/fibers/sched/go.hpp>
#include <exe/fibers/sched/yield.hpp>

#include <wheels/core/stop_watch.hpp>

#include <wheels/test/framework.hpp>

#include <thread>

using namespace exe;

void AssertOn(tp::ThreadPool& pool) {
  ASSERT_EQ(tp::ThreadPool::Current(), &pool);
}

TEST_SUITE(Fibers) {
  SIMPLE_TEST(JustWorks) {
    tp::ThreadPool pool{3};
    pool.Start();

    bool done = false;

    fibers::Go(pool, [&]() {
      AssertOn(pool);
      done = true;
    });

    pool.WaitIdle();

    ASSERT_TRUE(done);

    pool.Stop();
  }

  SIMPLE_TEST(Child) {
    tp::ThreadPool pool{3};
    pool.Start();

    std::atomic<size_t> done{0};

    auto init = [&]() {
      AssertOn(pool);

      fibers::Go([&]() {
        AssertOn(pool);
        ++done;
      });

      ++done;
    };

    fibers::Go(pool, init);

    pool.WaitIdle();

    ASSERT_EQ(done.load(), 2);

    pool.Stop();
  }

  SIMPLE_TEST(RunInParallel) {
    tp::ThreadPool pool{3};
    pool.Start();

    std::atomic<size_t> completed{0};

    auto sleeper = [&]() {
      std::this_thread::sleep_for(3s);
      completed.fetch_add(1);
    };

    wheels::StopWatch stop_watch;

    fibers::Go(pool, sleeper);
    fibers::Go(pool, sleeper);
    fibers::Go(pool, sleeper);

    pool.WaitIdle();

    ASSERT_EQ(completed.load(), 3);
    ASSERT_TRUE(stop_watch.Elapsed() < 3s + 500ms);

    pool.Stop();
  }

  SIMPLE_TEST(Yield) {
    std::atomic<int> value{0};

    auto check_value = [&]() {
      const int kLimit = 10;

      ASSERT_TRUE(value.load() < kLimit);
      ASSERT_TRUE(value.load() > -kLimit);
    };

    static const size_t kIterations = 12345;

    auto bull = [&]() {
      for (size_t i = 0; i < kIterations; ++i) {
        value.fetch_add(1);
        fibers::Yield();
        check_value();
      }
    };

    auto bear = [&]() {
      for (size_t i = 0; i < kIterations; ++i) {
        value.fetch_sub(1);
        fibers::Yield();
        check_value();
      }
    };

    // NB: 1 worker thread!
    tp::ThreadPool pool{1};
    pool.Start();

    fibers::Go(pool, [&]() {
      fibers::Go(bull);
      fibers::Go(bear);
    });

    pool.WaitIdle();
    pool.Stop();
  }

  SIMPLE_TEST(Yield2) {
    tp::ThreadPool pool{4};
    pool.Start();

    static const size_t kYields = 65'536;

    auto yielder = [] {
      for (size_t i = 0; i < kYields; ++i) {
        fibers::Yield();
      }
    };

    fibers::Go(pool, yielder);
    fibers::Go(pool, yielder);

    pool.WaitIdle();
    pool.Stop();
  }

  SIMPLE_TEST(TwoPools1) {
    tp::ThreadPool pool_1{4};
    pool_1.Start();

    tp::ThreadPool pool_2{4};
    pool_2.Start();

    auto make_tester = [](tp::ThreadPool& pool) {
      return [&pool]() {
        AssertOn(pool);
      };
    };

    fibers::Go(pool_1, make_tester(pool_1));
    fibers::Go(pool_2, make_tester(pool_2));

    pool_1.WaitIdle();
    pool_2.WaitIdle();

    pool_1.Stop();
    pool_2.Stop();
  }

  SIMPLE_TEST(TwoPools2) {
    tp::ThreadPool pool_1{4};
    pool_1.Start();

    tp::ThreadPool pool_2{4};
    pool_2.Start();

    auto make_tester = [](tp::ThreadPool& pool) {
      return [&pool]() {
        static const size_t kIterations = 1024;

        for (size_t i = 0; i < kIterations; ++i) {
          AssertOn(pool);

          fibers::Yield();

          fibers::Go(pool, [&pool]() {
            AssertOn(pool);
          });
        }
      };
    };

    fibers::Go(pool_1, make_tester(pool_1));
    fibers::Go(pool_2, make_tester(pool_2));

    pool_1.WaitIdle();
    pool_2.WaitIdle();

    pool_1.Stop();
    pool_2.Stop();
  }
}

RUN_ALL_TESTS()
