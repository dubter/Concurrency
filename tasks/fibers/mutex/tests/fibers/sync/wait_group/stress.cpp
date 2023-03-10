#include <wheels/test/framework.hpp>
#include <twist/test/budget.hpp>

#include <twist/test/with/wheels/stress.hpp>

#include <exe/executors/thread_pool.hpp>
#include <exe/fibers/sched/go.hpp>
#include <exe/fibers/sync/wait_group.hpp>

#include <atomic>
#include <chrono>

using namespace exe;
using namespace std::chrono_literals;

//////////////////////////////////////////////////////////////////////

void StressTest(size_t workers, size_t waiters) {
  executors::ThreadPool scheduler{/*threads=*/4};
  scheduler.Start();

  while (twist::test::KeepRunning()) {
    fibers::WaitGroup wg;

    std::atomic<size_t> waiters_done{0};
    std::atomic<size_t> workers_done{0};

    wg.Add(workers);

    // Waiters

    for (size_t i = 0; i < waiters; ++i) {
      fibers::Go(scheduler, [&] {
        wg.Wait();
        waiters_done.fetch_add(1);
      });
    }

    // Workers

    for (size_t j = 0; j < workers; ++j) {
      fibers::Go(scheduler, [&] {
        workers_done.fetch_add(1);
        wg.Done();
      });
    }

    scheduler.WaitIdle();

    ASSERT_EQ(waiters_done.load(), waiters);
    ASSERT_EQ(workers_done.load(), workers);
  }

  scheduler.Stop();
}

//////////////////////////////////////////////////////////////////////

TEST_SUITE(WaitGroup) {
  TWIST_TEST(Stress_1_1, 5s) {
    StressTest(/*workers=*/1, /*waiters=*/1);
  }

  TWIST_TEST(Stress_1_2, 5s) {
    StressTest(/*workers=*/2, /*waiters=*/2);
  }

  TWIST_TEST(Stress_1_3, 5s) {
    StressTest(/*workers=*/3, /*waiters=*/1);
  }
}

RUN_ALL_TESTS()
