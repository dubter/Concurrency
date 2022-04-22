#include <exe/executors/tp/compute/thread_pool.hpp>
#include <exe/fibers/core/api.hpp>
#include <exe/fibers/sync/mutex.hpp>
#include <exe/fibers/sync/wait_group.hpp>

#include <wheels/test/test_framework.hpp>

#include <twist/test/test.hpp>

#include <iostream>
#include <mutex>

using namespace exe;

TEST_SUITE(FibersOverExecutors) {
  TWIST_TEST_TL(ComputeThreadPool, 10s) {
    // Good old ThreadPool
    executors::tp::compute::ThreadPool scheduler{/*threads=*/4};

    bool done = false;

    fibers::Go(scheduler, [&done]() {
      fibers::WaitGroup wg;

      fibers::Mutex mutex;
      size_t cs = 0;

      static const size_t kFibers = 7;
      static const size_t kSectionsPerFiber = 100'000;

      wg.Add(kFibers);

      for (size_t i = 0; i < kFibers; ++i) {
        fibers::Go([&]() {
          for (size_t j = 0; j < kSectionsPerFiber; ++j) {
            std::lock_guard guard(mutex);
            ++cs;
          }
          wg.Done();
        });
      }

      wg.Wait();

      std::cout << "# critical sections: " << cs << std::endl;

      ASSERT_EQ(cs, kFibers * kSectionsPerFiber);
      done = true;
    });

    scheduler.WaitIdle();

    ASSERT_TRUE(done);

    scheduler.Stop();
  }
}

RUN_ALL_TESTS()
