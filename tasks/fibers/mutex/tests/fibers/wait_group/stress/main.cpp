#include <wheels/test/test_framework.hpp>
#include <wheels/test/util.hpp>

#include <twist/test/test.hpp>

#include <exe/tp/thread_pool.hpp>
#include <exe/fibers/sync/wait_group.hpp>

#include <atomic>
#include <chrono>

using namespace exe;
using namespace std::chrono_literals;

//////////////////////////////////////////////////////////////////////

void StressTest(size_t workers, size_t waiters) {
  tp::ThreadPool scheduler{/*threads=*/4};

  while (wheels::test::KeepRunning()) {
    fibers::WaitGroup wg;

    // Number of completed waiters
    std::atomic<size_t> oks{0};

    wg.Add(workers);

    // Waiters

    for (size_t i = 0; i < waiters; ++i) {
      fibers::Go(scheduler, [&]() {
        wg.Wait();
        ++oks;
      });
    }

    // Workers

    for (size_t j = 0; j < workers; ++j) {
      fibers::Go(scheduler, [&]() {
        wg.Done();
      });
    }

    scheduler.WaitIdle();

    ASSERT_EQ(oks, waiters);
  }

  scheduler.Stop();
}

//////////////////////////////////////////////////////////////////////

TEST_SUITE(WaitGroup) {
  TWIST_TEST_TL(Stress_1, 5s) {
    StressTest(/*workers=*/1, /*waiters=*/1);
  }

  TWIST_TEST_TL(Stress_2, 5s) {
    StressTest(/*workers=*/2, /*waiters=*/2);
  }

  TWIST_TEST_TL(Stress_3, 5s) {
    StressTest(/*workers=*/3, /*waiters=*/1);
  }
}

RUN_ALL_TESTS()
